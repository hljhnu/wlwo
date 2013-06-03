#include "global.h"
char wl_method[20]="security_refresh";//
//unsigned int line_size=64;
//#define  pcm_size 1048576;//unit:cacheline
unsigned int pivot=16777216*0.4;//pivot is the limitation between pcm space and back space;
unsigned long long total_write_count=0;
unsigned int total_write_count2=0;//requested write access count, exclude write on refreshing
unsigned int exceed_write_count=0;//number of address of write access beyond  1GB
unsigned int wear_out_count=0;
unsigned int access_count=0;//read from trace
unsigned int trace_data[700000];
unsigned int trace_len=0;
unsigned int pointer_deepth[pcm_size];

unsigned int access_path[1000]; //in case that access path forms a cycly.
unsigned int access_depth=0;

char trace[100] = "D:\\programs\\WLWO\\WLWO\\trace-LU.out";
char random[100] = "D:\\programs\\WLWO\\WLWO\\24bits_randomized_addr.dat";
char result_path[100]="D:\\programs\\WLWO\\WLWO\\result.txt";
bool pointer_printed[10]={false,false,false,false,false,false,false,false,false,false};

bool check_cycle(unsigned int start_address)
{
    unsigned int i=0;
    for(i=0;i<access_depth-1;i++)
    {
        if(access_path[access_depth-1]==access_path[i])
        {
            return false;
        }
    }
    return true;
}

