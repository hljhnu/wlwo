#include "global.h"
char wl_method[20]="security_refresh";//start_gap
unsigned int pivot=pcm_size*1;//pivot is the limitation between pcm space and back space;
unsigned long long total_write_count=0;
unsigned int first_broken_write_count=0;
unsigned int total_write_count2=0;//requested write access count, exclude write on refreshing
unsigned int exceed_write_count=0;//number of address of write access beyond  1GB
unsigned int wear_out_count=0;
unsigned int access_count=0;//read from trace
unsigned int last_written_line=0;//the last written line before the comming wear-leveling.
unsigned int trace_data[10000000];
unsigned int trace_len=0;
unsigned int pointer_deepth[pcm_size];
unsigned long long total_access_delay=0;
unsigned int birthday_random_address[20000];//chosen addresses for birthday paradox attack, 20000 is large enough for 4GB memory.
unsigned int access_hops[pcm_size];//recording access hops of every request.

unsigned int access_path[1000]; //in case that access path forms a cycle.
unsigned int access_depth=0;//used for testing cycle in access path.

char trace[100] = "trace-LU.out";/*  D:\\programs\\WLWO\\WLWO\\  */
char random[100] = "24bits_randomized_addr.dat";
char result_path[100]="result.txt";
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



