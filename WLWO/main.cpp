/*****************************************************
This file includes entrance "main" and the framwork of
the whole work.
******************************************************/
#include <iostream>
#include <ostream>
#include <fstream>
#include <iomanip>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <ctime>
#include "space.h"
#include "global.h"
#include "startgap.h"
#include "securityrefresh.h"
using namespace std;

ofstream outfile(result_path,ofstream::out|ofstream::app);

/** \brief access_line: This function is the main framwork of the whole work.
 *
 * \param  line_address: The address of a line from the last level cache to the memory.
 * \param  start_line_address:the head of an access path
 * \param  is_start: whether param line_address is the head of the access path
 * \param  update: access happens when perfrom security refreshing
 * \param  depth: the depth of pointer which points to line_address
 * \return bool: A return "true" indicates the success of the access.
 *
 */
bool access_line(unsigned int line_address,unsigned int start_line_address,bool is_start,bool update,int depth)//update:whether to update pointer depth
{
    total_access_delay+=50;
    if((line_address==start_line_address)&&(false==is_start))
    {
        return true;
    }
    is_start=false;
    //we do not need to consider read access.
    if((strcmp(wl_method,"start_gap")==0))
    {
        line_address=random_map[line_address];
    }
    unsigned int mapped_address = wear_leveling_map(line_address,wl_method,false);

    if(update)
    {
        //pointer_cache.invalid(mapped_address);
        pcm.lines[mapped_address].point_deep=depth+1;
    }
    if(pcm.lines[mapped_address].dpflag)//if dpflag == true , it is data in that cacheline.
    {
        if(pcm.lines[mapped_address].write_count>=pcm.lines[mapped_address].lifetime)
        {
            if(0==first_broken_write_count)
            {
                first_broken_write_count=total_write_count;
            }
            wear_out_count++;
            unsigned int re_mapped_address;
            bool success=remapping(mapped_address,&re_mapped_address);//A failure block is remapped to a logical address

            if(success)
            {
                unsigned int new_mapped_address=wear_leveling_map(re_mapped_address,wl_method,update);
                if(start_line_address<=pivot)//
                {
                    pcm.lines[new_mapped_address].point_deep=1;
                    unsigned int start_mapped_address = wear_leveling_map(start_line_address,wl_method,false);
                    unsigned int start_remapped_address=pcm.lines[start_mapped_address].remap_address;
                    pcm.lines[start_mapped_address].remap_address=re_mapped_address;
                    pcm.lines[mapped_address].remap_address=start_remapped_address;
                }
#ifdef POINTER_CACHE
                //bool in_cache=false;
                //in_cache=Reverse_pointer_cache.lookup(target_address);
                unsigned int target_address = wear_leveling_map(re_mapped_address,wl_method,update);
                if((false==update)&&(start_line_address<=pivot))
                {
                    pointer_cache.insert_entry(start_line_address,target_address);
                    reverse_pointer_cache.insert_entry(start_line_address,target_address);
                }
#endif
                return access_line(re_mapped_address,start_line_address,is_start,update,depth+1);
                //perform_access_pcm(re_mapped_address);
                //wear_leveling("start_gap");
            }
            else
            {
                return false;
            }
        }
        else
        {
#ifdef POINTER_CACHE
        /*
            if((false==update)&&(start_line_address<=pivot))
            {
                pointer_cache.insert_entry(start_mapped_address,pcm.lines[mapped_address].remap_address);
            }
        */
#endif
            if(false==update)
            {
                perform_access_pcm(mapped_address,update);
            }
            if(false==update)
            {
                access_hops[depth]++;
            }
#ifdef PRINT_POINTER_DEPTH
            int percent=wear_out_count/(pcm_size*0.1);
            if((pointer_printed[percent]==false)&&(wear_out_count>0)&&((wear_out_count%(pcm_size/10)==0)))
            {
                pointer_printed[percent]=true;
                print_pointer();
            }
#endif
            return true;
            //wear_leveling("start_gap");
        }
    }
    else// dp == false , it is a pointer in that line.
    {
        unsigned int remapped_address;
#ifdef POINTER_CACHE
        if((false==update)&&(start_line_address<=pivot))
        {
            unsigned int target_address;
            bool found=false;
            found=pointer_cache.lookup(start_line_address,&target_address);
            if(found)
            {
               //remapped_address = target_address;
               perform_access_pcm(target_address,update);
               return true;
            }
            else
            {
                remapped_address=pcm.lines[mapped_address].remap_address;
            }
        }
        else
        {
            remapped_address=pcm.lines[mapped_address].remap_address;
        }
#else
            remapped_address=pcm.lines[mapped_address].remap_address;
#endif
        return access_line(remapped_address,start_line_address,is_start,update,depth+1);
    }
}
bool access_address(unsigned int memory_address,bool update,int depth)
{
    return access_line(memory_address/line_size,memory_address/line_size,true,update,depth);
}

unsigned int write_count_sum[pcm_size];
unsigned int overlay()//in order to reduce runing time, we overlay write count before wear-out.
{
                       //it is used for security refresh.
    while(true)
    {
        unsigned int i;
         for(i=0;i<=pivot;i++)
         {
            unsigned int xor_address;
            unsigned int match_address;
            xor_address=xor_map(i<<line_bit_number,OUT_LEFT,OUT_RIGHT,kc);
            xor_address=xor_map(xor_address,OUT_LEFT,OUT_RIGHT,kp);
            match_address=xor_address>>line_bit_number;
            write_count_sum[i]+=pcm.lines[match_address].write_count;
            cout<<write_count_sum[i]<<endl;
            if(write_count_sum[i]>=pcm.lines[i].lifetime)
            {
                return i;
            }
        }
        kp=kc;
        kc=rand()%0xffffffff;
    }
}
void print_hops(ostream & outfile)//print number of different hops of access
{
    unsigned int i;
    cout<<endl;
    cout<<"access hops:"<<endl;
    outfile<<endl;
    outfile<<"access hops:"<<endl;
    for(i=0;i<pcm_size;i++)
    {
        if(access_hops[i]>0)
        {
            cout<<"hops: "<<i<<" ; count: "<<access_hops[i]<<endl;
            outfile<<"hops: "<<i<<" ; count: "<<access_hops[i]<<endl;
        }
    }
    cout<<endl;
    outfile<<endl;
}
void print_pointer()
{
    unsigned int i = 0;
    compute_pointer_depth();
    if(outfile.good())
    {
        outfile<<"\nwear-out percent: "<<fixed<<setprecision(1)<<((float)wear_out_count/(float)pcm_size)<<endl;
        for(i=0;i<=deepest_point;i++)
        {
            if(pointer_deepth[i]>0)
            {
                    outfile<<"pointer depth = "<<i<<"  ; count = "<<pointer_deepth[i]<<endl;
            }
        }
    }
}
unsigned int access_from_file(char * filename)
{
    cout<<"trace file: "<<filename<<endl;
    if(outfile.is_open())
    {
        outfile<<"trace file: "<<filename<<endl;
    }
    ifstream trace(filename);
    if(trace.is_open()==false)
    {
        cout<<"error:opening trace file "<<filename<<endl;
        return 0;
    }
    while(trace.eof()==false)
    {
        trace>>trace_data[trace_len];
        trace_len++;
    }
    trace_len--;//the last one is the end position of the file.
    cout<<"trace length= "<<trace_len<<endl;
    if(trace.is_open())
    {
        trace.close();
    }
    trace_len--;//in case the last one is a counter,rather than an address.

        unsigned int address;
        unsigned int i=0;
        bool successful=true;
        while(successful)
        {
          //  if( (inner_write_count[0]>100000000) || (region_write_count[0]>100000000) )
           // {
           //     break;
           // }

/*          if(total_write_count>RUN_LENGTH)
            {
                break;
            }
*/
            access_count++;
            address=trace_data[i];
            i=(i+1)%trace_len;
            //i=i+1;

            //cout<<address<<" ";
            if((address>>line_bit_number)>pivot)//addresss larger than pivot are used for remapping and are unvisible to OS.
            {
                //cout<<"\naccessed line address "<<address/line_size<<" is larger than size of pcm : "<<pivot<<endl;
                //return access_count;

#ifdef ADDRESS_NARROWED
                address=address%(pivot<<line_bit_number);
#else
                exceed_write_count++;
                continue;
#endif
            }
//#define FILTER
#ifdef FILTER
            /*filter the abnormal(too many) accesses to a block*/
            if((address>>line_bit_number)==9535)
            {
                continue;
            }
#endif
            successful=access_address(address,false,0);
            //if(total_write_count%1000==0)
                //cout<<" total write count: "<<total_write_count<<endl;
            if(successful==false)
            {
                cout<<"The device is wear-out!"<<endl;
                break;
            }

            total_write_count++;

            if(wear_leveling(wl_method)==false)
            {
                cout<<"The device is wear-out!"<<endl;
                break;
            }
        }
    return access_count;
}

unsigned int repeatation_attack()//repetation address attack
{
    cout<<"***********"<<"repeatation attack"<<"***********"<<endl;
    if(outfile.is_open())
    {
        outfile<<"***********"<<"repeatation attack"<<"***********"<<endl;
    }
    unsigned int address=rand()%pivot;
    address=address<<line_bit_number;
    bool successful=true;
    while(successful)
    {
        access_count++;
        successful=access_address(address,false,0);
        //if(total_write_count%1000==0)
            //cout<<" total write count: "<<total_write_count<<endl;
        if(successful==false)
        {
            cout<<"The device is wear-out!"<<endl;
            break;
        }

        total_write_count++;

        if(wear_leveling(wl_method)==false)
        {
            cout<<"The device is wear-out!"<<endl;
            break;
        }
    }
    return access_count;
}

unsigned int birthday_attack(unsigned int limit)//birthday paradox attack
{
#define PI 3.14
    cout<<"***********"<<"birthday paradox attack"<<"***********"<<endl;
    if(outfile.is_open())
    {
        outfile<<"***********"<<"birthday paradox attack"<<"***********"<<endl;
    }
    unsigned int result_set_count=sqrt((double)((PI/2)*pcm_size))+1;
    unsigned int temp_count=0;
    while(temp_count<result_set_count)
    {
        unsigned int address=rand()%pivot;
        unsigned int i=0;
        for(i=0;i<temp_count;i++)
        {
            if(address==birthday_random_address[i])
            {
                break;
            }
        }
        birthday_random_address[temp_count]=address;
        temp_count++;
    }
    bool successful=true;
    unsigned int i=0;
    while(successful)
    {
        access_count++;
        successful=access_address(birthday_random_address[i]<<line_bit_number,false,0);
        i=(i+1)%result_set_count;
        if(successful==false)
        {
            cout<<"The device is wear-out!"<<endl;
            break;
        }

        total_write_count++;
/*
        if(total_write_count>limit)
        {
            break;
        }
*/
        if(wear_leveling(wl_method)==false)
        {
            cout<<"The device is wear-out!"<<endl;
            break;
        }
    }
    return access_count;
}

char * get_time()
{
    time_t t;
    t=time(NULL);
    char * time;
    time=ctime(&t);
    return time;
}
void compute_pointer_depth()
{
    //unsigned int deepest_point=0;
    unsigned int i = 0;
    for(i=0;i<pcm_size;i++)
    {
        if(deepest_point<pcm.lines[i].point_deep)
        {
            deepest_point=pcm.lines[i].point_deep;
        }
    }

    for(i=0;i<pcm_size;i++)
    {
        if(pcm.lines[i].dpflag==true)
        {
            pointer_deepth[pcm.lines[i].point_deep]++;
        }
    }

}
void output_result(ostream &outfile)
{
    if(outfile.good())
    {
        outfile<<get_time()<<endl;
        outfile<<"pointer cache: ";
#ifdef POINTER_CACHE
        outfile<<"yes"<<endl;
#else
        outfile<<"no"<<endl;
#endif // POINETR_CACHE
        outfile<<"random replacement sheme for pointer cache"<<endl;
        outfile<<"method:"<<wl_method<<endl;
        outfile<<"pcm size(line):"<<pcm_size<<endl;
#ifdef ADDRESS_NARROWED
        outfile<<"addresses have been narrowed."<<endl;
#endif // ADDRESS_NARROWED
#ifdef WL_WRITE
        outfile<<"include extra wear-leveling write"<<endl;
#endif // WL_WRITE
        if(strcmp(wl_method,"security_refresh")==0)
        {
            outfile<<"refresh interval:"<<refresh_requency<<endl;
            outfile<<"last crp:"<<(crp>>line_bit_number)<<endl;
            outfile<<"refresh count: "<<refresh_count<<endl;
            outfile<<"refresh round: "<<refresh_round<<endl;
        }
        if(strcmp(wl_method,"start_gap")==0)
        {
            outfile<<"frequency: "<<frequency1<<endl;
        }
        outfile<<"cell lifetime: "<<pcm.lines[0].lifetime<<endl;
        outfile<<"max depth: " << deepest_point<<endl;
        outfile<<"access count: "<<access_count<<endl;
        outfile<<"total write count: "<<total_write_count<<endl;
        outfile<<"total write count(without remapping): "<<first_broken_write_count<<endl;
        outfile<<"percent(without/with remapping): "<<fixed<<setprecision(4)<<((float)first_broken_write_count/(float)total_write_count)<<endl;
        outfile<<"wear-out count: "<<wear_out_count<<endl;
        outfile<<"exceeded write count: "<<exceed_write_count<<endl;
        //outfile<<"total access delay: "<<total_access_delay<<" ns"<<endl;
        outfile<<"average access delay: "<<(total_access_delay/total_write_count)<<" ns"<<endl;
        outfile<<"normal space : backup space = "<<pivot<<" : "<< (pcm_size-pivot)<<endl;
        outfile<<"backup space percent:"<<setprecision(2)<<(float)(pcm_size-pivot)/(float)pcm_size<<endl;
        outfile<<"wear-out percent: "<<fixed<<setprecision(4)<<((float)wear_out_count/(float)pcm_size)<<endl;
        unsigned int i;
        for(i=0;i<=deepest_point;i++)
        {
            if(pointer_deepth[i]>0)
            {
                outfile<<"pointer depth = "<<i<<"  ; count = "<<pointer_deepth[i]<<endl;
            }
        }
        outfile<<endl;
    }
}


void out_footprint(ostream &outfile)
{
    if(!outfile.good())
    {
        cout<<"file for footprint is not open"<<endl;
        return ;
    }
    unsigned int print_interval=COUNT_INTERVAL;
    unsigned int i,j;
    outfile<<"count interval: "<<COUNT_INTERVAL<<endl;
    //cout<<"count interval: "<<COUNT_INTERVAL<<endl;
    outfile<<"footprint"<<endl;
    //cout<<"footprint"<<endl;
    unsigned long sum=0;
    unsigned long count_larger_zero=0;
    //unsigned int groups[10000];
    for(i=0;i<pcm_size;i=i+print_interval)
    {
        unsigned int temp_count=0;
        for(j=0;j<print_interval;j++)
        {
            temp_count+=pcm.lines[i+j].write_count;
        }
        if(temp_count>0)
        {
            outfile<<i<<","<<temp_count<<endl;
            //cout<<i<<" "<<temp_count<<endl;
            groups[count_larger_zero]=temp_count;
            count_larger_zero++;
        }
        sum=sum+temp_count;
    }
    double avr=(double)sum/(double)count_larger_zero;
    double var=0.0;
    for(i=0;i<count_larger_zero;i++)
    {
        var=var+(groups[i]-avr)*(groups[i]-avr);
    }
    var=var/(double)count_larger_zero;
    double stdev=sqrt(var);
    double cov=stdev/avr;
    outfile<<"var: "<<var<<endl;
    outfile<<"stdev: "<<stdev<<endl;
    outfile<<"avr: "<<avr<<endl;
    outfile<<"cov: "<<cov<<endl;
    //cout<<"var: "<<var<<endl;
    //cout<<"stdev: "<<stdev<<endl;
    //cout<<"avr: "<<avr<<endl;
    //cout<<"cov: "<<cov<<endl;
    outfile<<endl;
}

void print_sr_round(ostream & outfile)
{
    int i;
    unsigned int sr_region=1<<REGION_BITS;
    outfile<<"refresh round: "<<endl;
    outfile<<"region number, round"<<endl;
    for(i=0;i<sr_region;i++)
    {
        outfile<<i<<","<<inner_refresh_round[i]<<endl;
    }
    outfile<<endl;

    outfile<<"refresh count: "<<endl;
    outfile<<"region number, count"<<endl;
    for(i=0;i<sr_region;i++)
    {
        outfile<<i<<","<<inner_refresh_count[i]<<endl;
    }
    outfile<<endl;
}

int main()
{
    cout << "WLWO begins ... " << endl;
    char method_name[3][20]={"none","security_refresh","start_gap"};
    char trace_name[9][50]={"pin-BARNES.out","pin-CHOLESKY.out","pin-C-LU.out",
                            "pin-FFT.out","pin-FMM.out","pin-NON-C-LU.out","pin-NSQUARED.out",
                            "pin-OCEAN.out","pin-WATER-SPATIAL.out"};

        int i;
        int chosen_method;
        do{
        cout<<"choose method"<<endl;
        for(i=0;i<3;i++)
        {
            cout<<i<<":"<<method_name[i]<<endl;
        }
        cin>>chosen_method;
        }while((chosen_method>2)||(chosen_method<0));
        strcpy(wl_method,method_name[chosen_method]);

        int chosen_trace=0;
        do{
        cout<<"choose traces"<<endl;
        for(i=0;i<9;i++)
        {
            cout<<i<<":"<<trace_name[i]<<endl;
        }
        cin>>chosen_trace;
        }while((chosen_trace>8)||(chosen_trace<0));
        strcat(trace,trace_name[chosen_trace]);

        if((strcmp(wl_method,"security_refresh")!=0)&&(strcmp(wl_method,"start_gap")!=0)&&(strcmp(wl_method,"none")!=0))
        {
            cout<<"non-existing wear-leveling method: "<<wl_method<<endl;
            return 0;
        }
        if(outfile.is_open()==false)
        {
            cout<<"Error: opening result file: "<<result_path<<endl;
            return 0;
        }
        if(strcmp(wl_method,"start_gap")==0)//random address for start_gap
        {
            if(false==init_region_start_gap())
            {
                return 0;
            }
        }
        else if(strcmp(wl_method,"security_refresh")==0)
        {
            init_security_refresh();
        }

        access_from_file(trace);
        //birthday_attack(700000000);
        //repeatation_attack();
        //output_result(outfile);

    char single_result_name[200];
    memset(single_result_name,'\0',200);
    strcpy(single_result_name,trace_name[chosen_trace]);
    strcat(single_result_name,"-");
    strcat(single_result_name,method_name[chosen_method]);
    strcat(single_result_name,"-");
    strcat(single_result_name,get_time());
    strcat(single_result_name,"-");
    strcat(single_result_name,".csv");
    char no_letters[10]={':','\0','\n','\\',' '};
    for(i=0;single_result_name[i]!='\0';i++)
    {
        int j;
        for(j=0;j<5;j++)
        {
            if(single_result_name[i]==no_letters[j])
            {
                single_result_name[i]='-';
            }
        }
    }
    ofstream single_result_file(single_result_name);

    //output_result(outfile);
#ifdef PRINT_POINTER_DEPTH
    print_pointer();
#endif // PRINT_POINTER_DEPTH
#ifdef PRINT_HOPS
    print_hops(outfile);
#endif
#ifdef PRINT_FOOTPRINT
    //out_footprint(outfile);
    if(single_result_file.is_open())
    {
        output_result(single_result_file);
        out_footprint(single_result_file);
    }
    else
    {
        cout<<"file for single result is not open"<<endl;
    }
#endif
    outfile<<"=============================================================================="<<endl;
    output_result(cout);
    //out_footprint(cout);
    //print_sr_round(single_result_file);

    if(outfile.is_open())
    {
        outfile.close();
    }
    if(single_result_file.is_open())
    {
        single_result_file.close();
    }
    cout << "WLWO ends!" << endl;
    return 0;
}
