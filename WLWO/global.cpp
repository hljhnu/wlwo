#include "global.h"
char wl_method[20]="security_refresh";
//unsigned int line_size=64;
//#define  pcm_size 1048576;//unit:cacheline
unsigned int pivot=16777216*0.999;//pivot is the limitation between pcm space and back space;
unsigned long long total_write_count=0;
unsigned int total_write_count2=0;
unsigned int exceed_write_count=0;//number of address of write access beyond  1GB
unsigned int wear_out_count=0;
unsigned int access_count=0;//read from trace
unsigned int trace_data[700000];
unsigned int trace_len=0;
unsigned int pointer_deepth[pcm_size];
/*
unsigned int wear_leveling_map(unsigned int line_address,char* method);
void wear_leveling(char* method);
bool remapping(unsigned int valid_address);
bool allocate_address(unsigned int valid_address,unsigned int * remapped_address);
void perform_access_pcm(unsigned int line_address);
*/