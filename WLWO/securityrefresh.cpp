#include "securityrefresh.h"
#include "space.h"
#include <math.h>
#include <iostream>
using namespace std;
unsigned int sub_region_bit=7;
unsigned int line_bit_number=log10(line_size)/log10(2);
unsigned int outer_map_bits=32-line_bit_number-2;//2^20
unsigned int inner_map_bits=outer_map_bits-sub_region_bit;//2^13
unsigned int outer_count[16777216];//(unsigned int)(pow(2.0,outer_map_bits))
unsigned int inner_count[131072];//(unsigned int)(pow(2.0,inner_map_bits))
unsigned int outer_up_limitation=0x3fffffc0;
unsigned int outer_down_limitation=0<<line_bit_number;
unsigned int refresh_count=0;
unsigned int refresh_round=0;
unsigned int kp=0,kc=(rand()%pcm_size)<<line_bit_number,crp=0;//used for security refresh: kp--previous key, kc--current key, cp--current position
unsigned int kp2,kc2,crp2;//used for security refresh on back device;
unsigned int refresh_requency=10;
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


unsigned int security_refresh_map(unsigned int line_address,bool refreshing)
{
    unsigned int mapped_byte_address;
    unsigned int mapped_line_address;
#define PRE_WL
#ifdef PRE_WL
    unsigned int byte_address=line_address<<line_bit_number;
    unsigned int exchanged_address;
    exchanged_address=xor_map(byte_address,29,6,kc);
    exchanged_address=xor_map(exchanged_address,29,6,kp);
    bool use_kc=(byte_address<crp)||(exchanged_address<crp);// line_address has been refreshed
    //use_kc=use_kc||((refreshing==true)&&((crp==(line_address<<line_bit_number))||(crp==exchanged_address)));//access the two address to be refreshed
    //use_kc=use_kc||((refreshing==false)&&((total_write_count%refresh_requency)==0));//crp is in the path of the address matched to crp;
    if(use_kc)// when refreshing, we need kc
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
        if(crp==outer_up_limitation)
        {
            refresh_round++;
            crp=outer_down_limitation;
            kp=kc;
            kc=(rand()%pcm_size)<<line_bit_number;
            return true;
        }
        else
        {
            crp=crp+line_size;
        }
        refresh_count++;
        unsigned int exchange_address;
        //We do not need to real data migration as we are simulating the process.
        //We only update the write count;
        exchange_address = xor_map(crp,29,6,kc);
        exchange_address = xor_map(exchange_address,29,6,kp);

        unsigned int mapped_address1=security_refresh_map(exchange_address>>line_bit_number,true);
        unsigned int mapped_address2=security_refresh_map(crp>>line_bit_number,true);
        int point_deepth1=pcm.lines[mapped_address1].point_deep-1;
        int point_deepth2=pcm.lines[mapped_address2].point_deep-1;

        //exchange data
        unsigned int crp_line=crp>>line_bit_number;
        unsigned int exchange_line_address=exchange_address>>line_bit_number;
        access_depth=0;
        if(!exchange_access_line(crp_line,crp_line,point_deepth1))return false;
        access_depth=0;
        if(!exchange_access_line(exchange_line_address,exchange_line_address,point_deepth2))return false;
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

bool exchange_access_line(unsigned int line_address,unsigned int start_line_address,int deepth)//update:whether to update pointer deepth
{

    //we do not need to consider read access.
    unsigned int mapped_address = wear_leveling_map(line_address,wl_method,true);
    pcm.lines[mapped_address].point_deep=deepth+1;
    if(pcm.lines[mapped_address].dpflag)//if dpflag == true , it is data in that cacheline.
    {
        if(pcm.lines[mapped_address].write_count>=pcm.lines[mapped_address].lifetime)
        {
            wear_out_count++;
            unsigned int re_mapped_address;
            bool success=remapping(mapped_address,&re_mapped_address);//A failure block is remapped to a logical address
            if(success)
            {
                return access_line(re_mapped_address,start_line_address,false,true,deepth+1);
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
            int percent=wear_out_count/(pcm_size*0.1);
            if((pointer_printed[percent]==false)&&(wear_out_count>0)&&((wear_out_count%(pcm_size/10)==0)))
            {
                pointer_printed[percent]=true;
                print_pointer();
            }
            return true;
            //wear_leveling("start_gap");
        }
    }
    else// dp == false , it is a pointer in that line.
    {
         return access_line(pcm.lines[mapped_address].remap_address,start_line_address,false,true,deepth+1);
    }
}
