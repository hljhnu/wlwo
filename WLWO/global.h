#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define PCM_CELL_ENDURANCE  (100000000)
#define PCM_SIZE_BITS (30)
#define line_bit_number (6)
#define line_size (1<<line_bit_number)
#define pcm_size (1<<PCM_SIZE_BITS>>line_bit_number) //16777216:2^30/64 ; 16384:2^20/64; 1048576:2^20; 2097152:2^27/64
                                    //total space:     16793600
#define OUT_LEFT (PCM_SIZE_BITS-1)
#define OUT_RIGHT line_bit_number
#define SUB_REGION_BITS (24)//21(SR) 24(SG) a sub-region is 2MB(SR) or 16MB(SG), that is bit number of a sub-region
#define REGION_BITS (PCM_SIZE_BITS-SUB_REGION_BITS)//bits of the number of regions
#define INNER_LEFT (SUB_REGION_BITS-1)
#define INNER_RIGHT line_bit_number
//#define POINTER_CACHE
//#define PRINT_POINTER_DEPTH
//#define PRINT_HOPS
#define WL_WRITE  //whether count extra wear-leveling writes.
#define PRINT_FOOTPRINT
#define RUN_LENGTH (100000000LLU)//(2000000000LLU)//the system stops after writting RUN_LENGTH times.
#define COUNT_INTERVAL (1<<0)  //(1<<11)  //the numbers of lines to be added up to a group.
#define ADDRESS_NARROWED
#define PRE_WL
#define DEBUG
extern char wl_method[];
extern unsigned int pivot;//pivot is the limitation between pcm space and back space;
extern unsigned long long total_write_count;
extern unsigned int first_broken_write_count;
extern unsigned int total_write_count2;
extern unsigned int exceed_write_count;
extern unsigned int wear_out_count;
extern unsigned int access_count;//read from trace
extern unsigned int last_written_line;
extern unsigned int trace_data[];
extern unsigned int trace_len;
extern unsigned int pointer_deepth[];
extern char trace[] ;
extern char random[] ;
extern char result_path[];
extern bool pointer_printed[];
extern unsigned long long total_access_delay;
extern unsigned int birthday_random_address[];
extern unsigned int access_hops[];
extern unsigned int groups[];
extern unsigned int access_path[];
extern unsigned int access_depth;
extern unsigned int deepest_point;
extern unsigned int wear_leveling_map(unsigned int line_address,char* method,bool update);
extern bool wear_leveling(char* method);
extern bool remapping(unsigned int valid_address,unsigned int * remapped_address);
extern bool allocate_address(unsigned int valid_address,unsigned int * remapped_address);
extern void perform_access_pcm(unsigned int line_address,bool update);
extern bool access_line(unsigned int line_address,unsigned int start_line_address,bool is_start,bool update, int depth);
extern bool access_address(unsigned int memory_address,bool update,int depth);
extern void print_pointer();
extern bool check_cycle();
extern void out_footprint();
extern void compute_pointer_depth();

extern unsigned int lookup_target(unsigned int line_address);//update:whether to update pointer depth
extern bool check_pointer_cycle(unsigned int line_address, unsigned int start_line_address,unsigned int depth);//update:whether to update pointer depth
#endif
