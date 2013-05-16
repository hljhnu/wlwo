#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define pcm_size (16777216) //16777216:2^30/64 ; 16384:2^20/64; 1048576:2^20
                                    //total space:     16793600
#define line_size 64
#define PRE_WL

extern char wl_method[20];
//extern unsigned int line_size;
extern unsigned int pivot;//pivot is the limitation between pcm space and back space;
extern unsigned long long total_write_count;
extern unsigned int total_write_count2;
extern unsigned int exceed_write_count;
extern unsigned int wear_out_count;
extern unsigned int access_count;//read from trace
extern unsigned int trace_data[];
extern unsigned int trace_len;
extern unsigned int pointer_deepth[];

extern unsigned int wear_leveling_map(unsigned int line_address,char* method);
extern bool wear_leveling(char* method);
extern bool remapping(unsigned int valid_address,unsigned int * remapped_address);
extern bool allocate_address(unsigned int valid_address,unsigned int * remapped_address);
extern void perform_access_pcm(unsigned int line_address);
extern bool access_line(unsigned int line_address);
extern bool access_address(unsigned int memory_address);

#endif
