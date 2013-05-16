#include "securityrefresh.h"
#include "space.h"
#include <math.h>

unsigned int sub_region_bit=7;
unsigned int line_bit_number=log10(line_size)/log10(2);
unsigned int outer_map_bits=32-line_bit_number-2;//2^20
unsigned int inner_map_bits=outer_map_bits-sub_region_bit;//2^13
unsigned int outer_count[16777216];//(unsigned int)(pow(2.0,outer_map_bits))
unsigned int inner_count[131072];//(unsigned int)(pow(2.0,inner_map_bits))
unsigned int outer_up_limitation=0x3fffffc0;
unsigned int outer_down_limitation=0x40;


unsigned int kp=0,kc=0,crp=1<<line_bit_number;//used for security refresh: kp--previous key, kc--current key, cp--current position
unsigned int kp2,kc2,crp2;//used for security refresh on back device;
unsigned int refresh_requency=100;
//unsigned int total_write_count=0;
unsigned int refresh_requency2=100;
//unsigned int total_write_count2=0;

unsigned int xor_map(unsigned int byte_address,unsigned int end, unsigned int start, unsigned int key)
{
    //unsigned int byte_address=line_address*line_size;
    unsigned int temp_address=byte_address;
    unsigned int temp_key=key;

    temp_address=temp_address<<(31-end);
    temp_address=temp_address>>(31-end);
    temp_address=temp_address>>start;
    temp_address=temp_address<<start;

    temp_key=temp_key<<(31-end);
    temp_key=temp_key>>(31-end);
    temp_key=temp_key>>start;
    temp_key=temp_key<<start;

    unsigned int result_address;
    result_address=byte_address-temp_address+(temp_address^temp_key);
    return result_address;
}


unsigned int security_refresh_map(unsigned int line_address)
{
    unsigned mapped_byte_address;
    unsigned int mapped_line_address;
#define PRE_WL
#ifdef PRE_WL
    unsigned int byte_address=line_address<<line_bit_number;
    unsigned int pre_matched_address=byte_address;
    pre_matched_address=xor_map(pre_matched_address,29,6,kc);
    pre_matched_address=xor_map(pre_matched_address,29,6,kp);
    if(byte_address<crp||pre_matched_address<crp)
    {
        mapped_byte_address=xor_map(byte_address,29,6,kc);
    }
    else
    {
        mapped_byte_address=xor_map(byte_address,29,6,kp);
    }

#else
//followings are not finished.

     if(total_write_count%refresh_requency==0)
     {
         if(line_address>=pivot)
         {
             if(line_address < cp || ( (line_address^kc^kp) < cp) )
             {
                 remapped_line_address = line_address^kc;//有错误。可能映射到额外区域
             }
             else
             {
                 remapped_line_address = line_address;
             }
             cp=(cp+1)%pivot;
         }
         else
         {
             line_address=line_address-pivot;
             if(line_address < cp2 || ((line_address^kc2^kp2) < cp2) )
             {
                 remapped_line_address = line_address^kc2;//有错误，可能映射到正常区域
             }
             else
             {
                 remapped_line_address = line_address;
             }
             cp2=(cp2+1)%(pcm_size-pivot);
         }
     }
#endif // PRE_WL
    mapped_line_address=mapped_byte_address>>line_bit_number;
    return mapped_line_address;
}

bool security_refresh()
{
#ifdef PRE_WL
    if(total_write_count%refresh_requency==0)
    {
        unsigned int exchange_address;
        //We do not need to real data migration as we are simulating the process.
        //We only update the write count;
        exchange_address = xor_map(crp,29,6,kc);
        exchange_address = xor_map(exchange_address,29,6,kp);
        if(!access_address(exchange_address))return false;
        if(!access_address(crp))return false;
        //pcm.lines[temp_address>>line_bit_number].write_count++;
        //pcm.lines[cp].write_count++;
        if(crp==outer_up_limitation)
        {
            crp=outer_down_limitation;
            kp=kc;
            kc=rand()%0xffffffff;
        }
        else
        {
            crp=crp+line_size;
        }
    }
#else

         unsigned int temp_address;
        //We do not need to real data migration as we are simulating the process.
        //We only update the write count;
        temp_address = cp2^kc2;//有错误，映射错误。
        pcm.lines[temp_address].write_count++;
        pcm.lines[cp2].write_count++;
        cp=(cp2+1)%pivot;

#endif
    return true;
}
