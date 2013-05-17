/*****************************************************
This file includes entrance "main" and the framwork of
the whole work.
******************************************************/
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdio>
#include <cstring>
#include "space.h"
#include "global.h"
#include "startgap.h"
#include "securityrefresh.h"
using namespace std;

/** \brief access_line: This function is the main framwork of the whole work.
 *
 * \param line_address: The address of a line from the last level cache to the memory.
 * \param
 * \return bool: A return "true" indicates the success of the access.
 *
 */
bool access_line(unsigned int line_address)//
{
    //cout<<line_address<<endl;
    //we do not need to consider read access.
    if((strcmp(wl_method,"start_gap")==0))//&&(line_address<=pivot)
    {
        line_address=random_map[line_address];
    }
    unsigned int mapped_address = wear_leveling_map(line_address,wl_method);
    if(pcm.lines[mapped_address].dpflag)//if dpflag == true , it is data in that cacheline.
    {
        if(pcm.lines[mapped_address].write_count>=pcm.lines[mapped_address].lifetime)
        {
            wear_out_count++;
            unsigned int re_mapped_address;
            bool success=remapping(mapped_address,&re_mapped_address);//A failure block is remapped to a logical address
            if(success)
            {
                return access_line(re_mapped_address);
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
            perform_access_pcm(mapped_address);
            return true;
            //wear_leveling("start_gap");
        }
    }
    else// dp == false , it is a pointer in that line.
    {
         return access_line(pcm.lines[mapped_address].remap_address);
    }
}
bool access_address(unsigned int memory_address)
{
    return access_line(memory_address/line_size);
}

unsigned int write_count_sum[pcm_size];
unsigned int overlay_threshold=pcm_size*refresh_requency;
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
            xor_address=xor_map(i<<line_bit_number,39,6,kc);
            xor_address=xor_map(xor_address,39,6,kp);
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
unsigned int access_from_file(char * filename)
{
    ifstream trace(filename);
    if(trace.is_open()==false)
    {
        cout<<"error:opening "<<filename<<endl;
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
 //trace_len--;the last one is a count,rather than an address.
    bool overlayed=false;
    bool pcm_wear_out=false;
    while(pcm_wear_out==false)
    {
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
            successful=access_address(address);
            //if(total_write_count%1000==0)
                //cout<<" total write count: "<<total_write_count<<endl;
            if(successful==false)
            {
                pcm_wear_out=true;
                cout<<"The device is wear out!"<<endl;
                break;
            }
            if((overlayed==false)&&(total_write_count==overlay_threshold))
            {
                overlayed=true;
                cout<<"overlay"<<endl;
                overlay();
            }
/*
            if(!wear_leveling(wl_method))
            {
                pcm_wear_out=true;
                cout<<"The device is wear out!"<<endl;
                break;
            }
*/
        }

    }
    return access_count;
}


void output_result()
{
    char result_path[50]="D:\\programs\\WLWO-optimized\\WLWO\\result.txt";
    ofstream outfile(result_path,ofstream::out|ofstream::app);
    if(outfile.is_open()==false)
    {
        cout<<"error: open"<<result_path<<endl;
        cout<<"result has not been saved!"<<endl;
    }
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

    cout<<"method:"<<wl_method<<endl;
    cout<<"cell lifetime: "<<pcm.lines[0].lifetime<<endl;
    cout<<"deepth of the deepest point: " << deepest_point<<endl;
    cout<<"access count: "<<access_count<<endl;
    cout<<"total write count: "<<total_write_count<<endl;
    cout<<"exceeded write count: "<<exceed_write_count<<endl;
    cout<<"normal space : backup space = "<<pivot<<" : "<< (pcm_size-pivot)<<" backup space percent:"<<setprecision(2)<<(float)(pcm_size-pivot)/(float)pcm_size<<endl;
    for(i=0;i<=deepest_point;i++)
    {
        if(pointer_deepth[i]>0)
        {
                cout<<"pointer deepth = "<<i<<"  ; count = "<<pointer_deepth[i]<<endl;
        }
    }
    if(outfile.is_open())
    {
        outfile<<"method:"<<wl_method<<endl;
        outfile<<"cell lifetime: "<<pcm.lines[0].lifetime<<endl;
        outfile<<"deepth of the deepest point: " << deepest_point<<endl;
        outfile<<"access count: "<<access_count<<endl;
        outfile<<"total write count: "<<total_write_count<<endl;
        outfile<<"exceeded write count: "<<exceed_write_count<<endl;
        outfile<<"normal space : backup space = "<<pivot<<" : "<< (pcm_size-pivot)<<" backup space percent:"<<setprecision(2)<<(float)(pcm_size-pivot)/(float)pcm_size<<endl;
        for(i=0;i<=deepest_point;i++)
        {
            if(pointer_deepth[i]>0)
            {
                outfile<<"pointer deepth = "<<i<<"  ; count = "<<pointer_deepth[i]<<endl;
            }
        }
        outfile<<endl;
        outfile.close();
    }
}
int main()
{
    cout << "WLWO begins ... " << endl;
    char * trace = "D:\\programs\\WLWO-optimized\\WLWO\\trace-LU.out";
    char * random = "D:\\programs\\WLWO-optimized\\WLWO\\24bits_randomized_addr.dat";
    if(strcmp(wl_method,"start_gap")==false)//random address for start_gap
    {
        ifstream random_map_file(random);
        if(random_map_file.is_open()==false)
        {
            cout<<"Errors happens when opening 24bits_randomized_addr.dat"<<endl;
            return 0;
        }
        unsigned int i=0;
        while(random_map_file.eof()==0)
        {
            random_map_file>>random_map[i];
            i++;
        }
        if(random_map_file.is_open())
        {
            random_map_file.close();
        }
    }

/*
    unsigned int i = 0;
    for(i=0;i<pivot;i++)//3850
    {
        access_line(i);
    }
    */
    unsigned long long access_count=0;
    access_count=access_from_file(trace);
    output_result();

    cout << "WLWO ends!" << endl;
    return 0;
}
