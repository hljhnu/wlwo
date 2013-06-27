/*****************************************************
This file includes entrance "main" and the framwork of
the whole work.
******************************************************/
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdio>
#include <cstring>
#include <cmath>
#include "space.h"
#include "global.h"
#include "startgap.h"
#include "securityrefresh.h"
using namespace std;

ofstream outfile(result_path,ofstream::out|ofstream::app);
#ifdef DEBUG
unsigned int a_access_count=0;
#endif // DEBUG


/** \brief access_line: This function is the main framwork of the whole work.
 *
 * \param  line_address: The address of a line from the last level cache to the memory.
 * \param  start_line_address:the head of an access path
 * \param  is_start: whether param line_address is the head of the access path
 * \param  update: access happens when perfrom security refreshing
 * \param  deepth: the deepth of pointer which points to line_address
 * \return bool: A return "true" indicates the success of the access.
 *
 */
bool access_line(unsigned int line_address,unsigned int start_line_address,bool is_start,bool update,int deepth)//update:whether to update pointer deepth
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
    //unsigned int start_mapped_address = wear_leveling_map(start_line_address,wl_method,false);
    if(update)
    {
        //pointer_cache.invalid(mapped_address);
        pcm.lines[mapped_address].point_deep=deepth+1;
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
                return access_line(re_mapped_address,start_line_address,is_start,update,deepth+1);
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
            perform_access_pcm(mapped_address);
            if(false==update)
            {
                access_hops[deepth]++;
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
               perform_access_pcm(target_address);
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
        return access_line(remapped_address,start_line_address,is_start,update,deepth+1);
    }
}
bool access_address(unsigned int memory_address,bool update,int deepth)
{
    return access_line(memory_address/line_size,memory_address/line_size,true,update,deepth);
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
void print_hops()//print number of different hops of access
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
    unsigned int deepest_point=0;
    unsigned int i = 0;
    for(i=0;i<pcm_size;i++)
    {
       /* if(pcm.lines[i].write_count>=0)
        {
            printf("%u : %u\n",i,pcm.lines[i].write_count);
        }*/
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
    cout<<"\nwear-out percent: "<<fixed<<setprecision(1)<<((float)wear_out_count/(float)pcm_size)<<endl;
#ifdef PRINT_POINTER_DEPTH
    for(i=0;i<=deepest_point;i++)
    {
        if(pointer_deepth[i]>0)
        {
                cout<<"pointer deepth = "<<i<<"  ; count = "<<pointer_deepth[i]<<endl;
        }
    }

    if(outfile.is_open())
    {
        outfile<<"\nwear-out percent: "<<fixed<<setprecision(1)<<((float)wear_out_count/(float)pcm_size)<<endl;
        for(i=0;i<=deepest_point;i++)
        {
            if(pointer_deepth[i]>0)
            {
                    outfile<<"pointer deepth = "<<i<<"  ; count = "<<pointer_deepth[i]<<endl;
            }
        }
    }
#endif
}
unsigned int access_from_file(char * filename)
{
    cout<<"trace file: "<<trace<<endl;
    if(outfile.is_open())
    {
        outfile<<"trace file: "<<trace<<endl;
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
            access_count++;
            address=trace_data[i];
            i=(i+1)%trace_len;

            //cout<<address<<" ";
            if((address>>line_bit_number)>pivot)//addresss larger than pivot are used for remapping and are unvisible to OS.
            {
                //cout<<"\naccessed line address "<<address/line_size<<" is larger than size of pcm : "<<pivot<<endl;
                //return access_count;
                exceed_write_count++;
                //address=address%pivot;
                continue;
            }
#define FILTER
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

void output_result()
{
    unsigned int deepest_point=0;
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

    cout<<"method:"<<wl_method<<endl;
    cout<<"pcm size(line):"<<pcm_size<<endl;
    if(strcmp(wl_method,"security_refresh")==0)
    {
        cout<<"refresh interval:"<<refresh_requency<<endl;
        cout<<"last crp:"<<(crp>>line_bit_number)<<endl;
        cout<<"refresh count: "<<refresh_count<<endl;
        cout<<"refresh round: "<<refresh_round<<endl;
    }
    cout<<"cell lifetime: "<<pcm.lines[0].lifetime<<endl;
    cout<<"deepth of the deepest point: " << deepest_point<<endl;
    cout<<"access count: "<<access_count<<endl;
    cout<<"total write count: "<<total_write_count<<endl;
    cout<<"total write count(without remapping): "<<first_broken_write_count<<endl;
    cout<<"percent(without/with remapping): "<<fixed<<setprecision(4)<<((float)first_broken_write_count/(float)total_write_count)<<endl;
    cout<<"wear-out count: "<<wear_out_count<<endl;
    cout<<"exceeded write count: "<<exceed_write_count<<endl;
    //cout<<"total access delay: "<<total_access_delay<<" ns"<<endl;
    cout<<"average access delay: "<<(total_access_delay/total_write_count)<<" ns"<<endl;
    cout<<"normal space : backup space = "<<pivot<<" : "<< (pcm_size-pivot)<<" backup space percent:"<<setprecision(2)<<(float)(pcm_size-pivot)/(float)pcm_size<<endl;
    cout<<"\nwear-out percent: "<<fixed<<setprecision(4)<<((float)wear_out_count/(float)pcm_size)<<endl;

    for(i=0;i<=deepest_point;i++)
    {
        if(pointer_deepth[i]>0)
        {
                cout<<"pointer deepth = "<<i<<"  ; count = "<<pointer_deepth[i]<<endl;
        }
    }
    if(outfile.is_open())
    {

#ifdef POINTER_CACHE
        outfile<<"***********"<<"with pointer cache"<<"***********"<<endl;
#else
        outfile<<"***********"<<"without pointer cache"<<"********"<<endl;
#endif // POINETR_CACHE
        outfile<<"************random replacement sheme for pointer cache*******"<<endl;
        outfile<<"method:"<<wl_method<<endl;
        outfile<<"pcm size(line):"<<pcm_size<<endl;
        if(strcmp(wl_method,"security_refresh")==0)
        {
            outfile<<"refresh interval:"<<refresh_requency<<endl;
            outfile<<"last crp:"<<(crp>>line_bit_number)<<endl;
            outfile<<"refresh count: "<<refresh_count<<endl;
            outfile<<"refresh round: "<<refresh_round<<endl;
        }
        outfile<<"cell lifetime: "<<pcm.lines[0].lifetime<<endl;
        outfile<<"deepth of the deepest point: " << deepest_point<<endl;
        outfile<<"access count: "<<access_count<<endl;
        outfile<<"total write count: "<<total_write_count<<endl;
        outfile<<"total write count(without remapping): "<<first_broken_write_count<<endl;
        outfile<<"percent(without/with remapping): "<<fixed<<setprecision(4)<<((float)first_broken_write_count/(float)total_write_count)<<endl;
        outfile<<"wear-out count: "<<wear_out_count<<endl;
        outfile<<"exceeded write count: "<<exceed_write_count<<endl;
        //outfile<<"total access delay: "<<total_access_delay<<" ns"<<endl;
        outfile<<"average access delay: "<<(total_access_delay/total_write_count)<<" ns"<<endl;
        outfile<<"normal space : backup space = "<<pivot<<" : "<< (pcm_size-pivot)<<" backup space percent:"<<setprecision(2)<<(float)(pcm_size-pivot)/(float)pcm_size<<endl;
        outfile<<"\nwear-out percent: "<<fixed<<setprecision(4)<<((float)wear_out_count/(float)pcm_size)<<endl;
        for(i=0;i<=deepest_point;i++)
        {
            if(pointer_deepth[i]>0)
            {
                outfile<<"pointer deepth = "<<i<<"  ; count = "<<pointer_deepth[i]<<endl;
            }
        }
        outfile<<endl;
    }

#ifdef PRINT_HOPS
    print_hops();
#endif
#ifdef PRINT_FOOTPRINT
    out_footprint();
#endif
    outfile<<"=============================================================================="<<endl;
}

void out_footprint()
{
    unsigned int print_interval=4096>>line_bit_number;
    unsigned int i,j;
    outfile<<"footprint"<<endl;
    cout<<"footprint"<<endl;
    for(i=0;i<pcm_size;i=i+print_interval)
    {
        unsigned int temp_count=0;
        for(j=0;j<print_interval;j++)
        {
            temp_count+=pcm.lines[i+j].write_count;
        }
        outfile<<i<<" "<<temp_count<<endl;
        cout<<i<<" "<<temp_count<<endl;
    }
    outfile<<endl;
}
int main()
{
    cout << "WLWO begins ... " << endl;
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
        if(false==init_start_gap())
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
    output_result();

    if(outfile.is_open())
    {
        outfile.close();
    }
    cout << "WLWO ends!" << endl;
    return 0;
}
