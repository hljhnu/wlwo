#ifndef _SECURITYREFRESH_H_
#define _SECURITYREFRESH_H_
#include "global.h"
//extern pcm_size;//unit:cacheline
#define OUT_LEFT 26
#define OUT_RIGHT 6
#define pointer_cache_size (8192)  // 2048-set 4-way
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
extern unsigned int refresh_count;
extern unsigned int refresh_round;
extern unsigned int xor_map(unsigned int byte_address,unsigned int end, unsigned int start, unsigned int key);
extern unsigned int security_refresh_map(unsigned int line_address,bool update);
extern bool security_refresh();
extern bool exchange_access_line(unsigned int line_address,unsigned int start_line_address,int deepth);
class Pointer_Cache
{
public:
    bool valid[pointer_cache_size];
    unsigned int source[pointer_cache_size];
    unsigned int target[pointer_cache_size];
    unsigned int ocupied_size;
    Pointer_Cache()
    {
        unsigned int i;
        for(i=0;i<pointer_cache_size;i++)
        {
            valid[i]=false;
            source[i]=0xffffffff;
            target[i]=0xffffffff;
        }
        ocupied_size=0;
    }
    bool slot_exist()
    {
        if(pointer_cache_size<=ocupied_size)
        {
            return false;
        }
    }
    bool insert_entry(unsigned int source_address, unsigned int target_address)
    {
        unsigned int cache_set = source_address%2048;
        unsigned int target_base = cache_set*4;
        unsigned int i;
        for(i=0;i<4;i++)
        {
            if(source_address==source[target_base+i])
            {
                target[target_base+i]=target_address;
                return true;
            }
        }

        for(i=0;i<4;i++)
        {
            if(false==valid[target_base+i])
            {
                source[target_base+i]=source_address;
                target[target_base+i]=target_address;
                valid[target_base+i]=true;
                return true;
            }
        }
        return false;
    }

    bool lookup(unsigned int source_address,unsigned int *target_address)
    {
        unsigned int cache_set = source_address%2048;
        unsigned int target_base = cache_set*4;
        unsigned int i;
        for(i=0;i<4;i++)
        {
            if((true==valid[target_base+i])&&(source[target_base+i]==source_address))
            {
                *target_address = target[target_base+i];
                return true;
            }
        }
        return false;  // do not find
    }
    bool invalid(unsigned int source_address)
    {
        unsigned int cache_set = source_address%2048;
        unsigned int target_base = cache_set*4;
        unsigned int i;
        for(i=0;i<4;i++)
        {
            if((true==valid[target_base+i])&&(source[target_base+i]==source_address))
            {
                valid[target_base+i]=false;
                return true;
            }
        }
        return false;  // do not find
    }

};

class Reverse_Pointer_Cache
{
public:
    bool valid[pointer_cache_size];
    unsigned int source[pointer_cache_size];
    unsigned int target[pointer_cache_size];
    unsigned int ocupied_size;
    Reverse_Pointer_Cache()
    {
        unsigned int i;
        for(i=0;i<pointer_cache_size;i++)
        {
            valid[i]=false;
            source[i]=0xffffffff;
            target[i]=0xffffffff;
        }
        ocupied_size=0;
    }
    bool slot_exist()
    {
        if(pointer_cache_size<=ocupied_size)
        {
            return false;
        }
    }
    bool insert_entry(unsigned int source_address, unsigned int target_address)
    {
        unsigned int cache_set = target_address%2048;
        unsigned int source_base = cache_set*4;
        unsigned int i;
        for(i=0;i<4;i++)
        {
            if(target_address==target[source_base+i])
            {
                source[source_base+i]=source_address;
                return true;
            }
        }

        for(i=0;i<4;i++)
        {
            if(false==valid[source_base+i])
            {
                source[source_base+i]=source_address;
                target[source_base+i]=target_address;
                valid[source_base+i]=true;
                return true;
            }
        }
        return false;
    }

    bool lookup(unsigned int *source_address,unsigned int target_address)
    {
        unsigned int cache_set = target_address%2048;
        unsigned int source_base = cache_set*4;
        unsigned int i;
        for(i=0;i<4;i++)
        {
            if((true==valid[source_base+i])&&(target[source_base+i]==target_address))
            {
                *source_address = source[source_base+i];
                return true;
            }
        }
        return false;  // do not find
    }
    bool invalid(unsigned int target_address)
    {
        unsigned int cache_set = target_address%2048;
        unsigned int source_base = cache_set*4;
        unsigned int i;
        for(i=0;i<4;i++)
        {
            if((true==valid[source_base+i])&&(target[source_base+i]==target_address))
            {
                valid[source_base+i]=false;
                return true;
            }
        }
        return false;  // do not find
    }

};

extern class Pointer_Cache pointer_cache;
extern class Reverse_Pointer_Cache reverse_pointer_cache;
#endif // _SECURITYREFRESH_H_
