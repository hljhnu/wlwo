#ifndef _SECURITYREFRESH_H_
#define _SECURITYREFRESH_H_
#include "global.h"
//extern pcm_size;//unit:cacheline
extern unsigned int sub_region_bit;
extern unsigned int line_bit_number;
extern unsigned int cache_line_bit;
extern unsigned int outer_map_bits;
extern unsigned int inner_map_bits;
extern unsigned int outer_up_limitation;
extern unsigned int outer_down_limitation;
extern unsigned int kp,kc,crp;//used for security refresh: kp--previous key, kc--current key, cp--current position
extern unsigned int kp2,kc2,crp2;//used for security refresh on back device;
extern unsigned int refresh_requency;
//extern unsigned int total_write_count;
extern unsigned int refresh_requency2;
//extern unsigned int total_write_count2;
extern unsigned int xor_map(unsigned int byte_address,unsigned int end, unsigned int start, unsigned int key);
extern unsigned int security_refresh_map(unsigned int line_address);
extern bool security_refresh();
#endif // _SECURITYREFRESH_H_
