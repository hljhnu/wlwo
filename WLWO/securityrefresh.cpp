#include "securityrefresh.h"
#include "space.h"
#include <math.h>
#include <iostream>
using namespace std;
unsigned int sub_region_bit=7;
unsigned int outer_map_bits=32-line_bit_number-2;//2^20
unsigned int inner_map_bits=outer_map_bits-sub_region_bit;//2^13
unsigned long long outer_up_limitation=(1LLU<<(OUT_LEFT+1))-line_size;
unsigned int outer_down_limitation=0<<line_bit_number;
unsigned int refresh_count=0;
unsigned int refresh_round=0;

unsigned long long inner_write_count[1<<REGION_BITS];
unsigned int inner_crp[1<<REGION_BITS];
unsigned int inner_kp[1<<REGION_BITS];
unsigned int inner_kc[1<<REGION_BITS];

unsigned int kp=0,kc=(rand()%pcm_size)<<line_bit_number,crp=0;//used for security refresh: kp--previous key, kc--current key, cp--current position
unsigned int kp2,kc2,crp2;//used for security refresh on back device;
unsigned int refresh_requency=100;
//unsigned int total_write_count=0;
unsigned int refresh_requency2=100;
//unsigned int total_write_count2=0;
class Pointer_Cache pointer_cache;
class Reverse_Pointer_Cache reverse_pointer_cache;

void init_security_refresh()
{
    unsigned int i;
    for(i=0;i<(1<<REGION_BITS);i++)
    {
        inner_crp[i]=i<<SUB_REGION_BITS;
    }
}

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

unsigned int sub_region_security_refresh_map(unsigned int line_address,bool refreshing)
{
    unsigned int region_number=line_address>>(SUB_REGION_BITS-line_bit_number);
    unsigned int mapped_byte_address;
    unsigned int mapped_line_address;
#define PRE_WL
#ifdef PRE_WL
    unsigned int byte_address=line_address<<line_bit_number;
    unsigned int exchanged_address;
    exchanged_address=xor_map(byte_address,INNER_LEFT,INNER_RIGHT,kc);
    exchanged_address=xor_map(exchanged_address,INNER_LEFT,INNER_RIGHT,kp);
    bool use_kc=(byte_address<inner_crp[region_number])||(exchanged_address<inner_crp[region_number]);// line_address has been refreshed
    //use_kc=use_kc||((refreshing==true)&&((crp==(line_address<<line_bit_number))||(crp==exchanged_address)));//access the two address to be refreshed
    //use_kc=use_kc||((refreshing==false)&&((total_write_count%refresh_requency)==0));//crp is in the path of the address matched to crp;
    if(use_kc)// when refreshing, we need kc
    {
        mapped_byte_address=xor_map(byte_address,INNER_LEFT,INNER_RIGHT,inner_kc[region_number]);
    }
    else
    {
        mapped_byte_address=xor_map(byte_address,INNER_LEFT,INNER_RIGHT,inner_kp[region_number]);
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

unsigned int security_refresh_map(unsigned int line_address,bool refreshing)
{
    unsigned int mapped_byte_address;
    unsigned int mapped_line_address;
#define PRE_WL
#ifdef PRE_WL
    unsigned int byte_address=line_address<<line_bit_number;
    unsigned int exchanged_address;
    exchanged_address=xor_map(byte_address,OUT_LEFT,OUT_RIGHT,kc);
    exchanged_address=xor_map(exchanged_address,OUT_LEFT,OUT_RIGHT,kp);
    bool use_kc=(byte_address<crp)||(exchanged_address<crp);// line_address has been refreshed
    //use_kc=use_kc||((refreshing==true)&&((crp==(line_address<<line_bit_number))||(crp==exchanged_address)));//access the two address to be refreshed
    //use_kc=use_kc||((refreshing==false)&&((total_write_count%refresh_requency)==0));//crp is in the path of the address matched to crp;
    if(use_kc)// when refreshing, we need kc
    {
        mapped_byte_address=xor_map(byte_address,OUT_LEFT,OUT_RIGHT,kc);
    }
    else
    {
        mapped_byte_address=xor_map(byte_address,OUT_LEFT,OUT_RIGHT,kp);
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
        unsigned int old_target1,old_target2;
        unsigned int new_target1,new_target2;
        bool old_cycle1,old_cycle2;
        bool new_cycle1,new_cycle2;
        unsigned int exchange_address;

        //We do not need to real data migration as we are simulating the process.
        //We only update the write count;
        exchange_address = xor_map(crp,OUT_LEFT,OUT_RIGHT,kc);
        exchange_address = xor_map(exchange_address,OUT_LEFT,OUT_RIGHT,kp);

        unsigned int mapped_address1=security_refresh_map(exchange_address>>line_bit_number,true);
        unsigned int mapped_address2=security_refresh_map(crp>>line_bit_number,true);
        int point_deepth1=pcm.lines[mapped_address1].point_deep-1;
        int point_deepth2=pcm.lines[mapped_address2].point_deep-1;
        old_cycle1=check_pointer_cycle(exchange_address>>line_bit_number,exchange_address>>line_bit_number,0);
        old_cycle2=check_pointer_cycle(crp>>line_bit_number,crp>>line_bit_number,0);
        if(!old_cycle1)
        {
            old_target1=lookup_target(exchange_address>>line_bit_number);
        }
        if(!old_cycle2)
        {
            old_target2=lookup_target(crp>>line_bit_number);
        }

        refresh_count++;


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
        new_cycle1=check_pointer_cycle(exchange_address>>line_bit_number,exchange_address>>line_bit_number,0);
        new_cycle2=check_pointer_cycle(crp>>line_bit_number,crp>>line_bit_number,0);
        new_target1=old_target2;
        new_target2=old_target1;
        if(old_cycle1||old_cycle2||new_cycle1||new_cycle2)
        {
            return true;
        }
        unsigned int remapped_address1,remapped_address2;
        bool remap_success1,remap_success2;
        if(pcm.lines[new_target1].write_count>=pcm.lines[new_target1].lifetime)//the block to be refreshed has worn out
        {
            remap_success1=remapping(new_target1,&remapped_address1);
            if(remap_success1)
            {
                new_target1=security_refresh_map(remapped_address1,true);
            }
            else
            {
                return false;
            }
        }
        if(pcm.lines[new_target2].write_count>=pcm.lines[new_target2].lifetime)//the block to be refreshed has worn out
        {
            remap_success2=remapping(new_target2,&remapped_address2);
            if(remap_success2)
            {
                new_target2=security_refresh_map(remapped_address2,true);
            }
            else
            {
                return false;
            }
        }

#ifdef POINTER_CACHE
        unsigned int source_address1;
        unsigned int source_address2;
        if(reverse_pointer_cache.lookup(&source_address1,new_target1))
        {
            pointer_cache.insert_entry(source_address1,new_target1);
            reverse_pointer_cache.insert_entry(source_address1,new_target1);
        }
        if(reverse_pointer_cache.lookup(&source_address2,new_target2))
        {
            pointer_cache.insert_entry(source_address2,new_target2);
            reverse_pointer_cache.insert_entry(source_address2,new_target2);
        }
#endif
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

bool sub_region_security_refresh()
{
#ifdef PRE_WL
    unsigned int last_written_sub_region=last_written_line>>(SUB_REGION_BITS-line_bit_number);
    inner_write_count[last_written_sub_region]++;
    if(inner_write_count[last_written_sub_region]%refresh_requency==0)
    {
        unsigned int old_target1,old_target2;
        unsigned int new_target1,new_target2;
        bool old_cycle1,old_cycle2;
        bool new_cycle1,new_cycle2;
        unsigned int exchange_address;

        //We do not need to real data migration as we are simulating the process.
        //We only update the write count;
        exchange_address = xor_map(inner_crp[last_written_sub_region],INNER_LEFT,INNER_RIGHT,inner_kc[last_written_sub_region]);
        exchange_address = xor_map(exchange_address,INNER_LEFT,INNER_RIGHT,inner_kp[last_written_sub_region]);

        unsigned int mapped_address1=security_refresh_map(exchange_address>>line_bit_number,true);
        unsigned int mapped_address2=security_refresh_map(inner_crp[last_written_sub_region]>>line_bit_number,true);
        int point_deepth1=pcm.lines[mapped_address1].point_deep-1;
        int point_deepth2=pcm.lines[mapped_address2].point_deep-1;
        old_cycle1=check_pointer_cycle(exchange_address>>line_bit_number,exchange_address>>line_bit_number,0);
        old_cycle2=check_pointer_cycle(inner_crp[last_written_sub_region]>>line_bit_number,inner_crp[last_written_sub_region]>>line_bit_number,0);
        if(!old_cycle1)
        {
            old_target1=lookup_target(exchange_address>>line_bit_number);
        }
        if(!old_cycle2)
        {
            old_target2=lookup_target(inner_crp[last_written_sub_region]>>line_bit_number);
        }

        refresh_count++;

        unsigned int this_up_limitation=(last_written_sub_region<<SUB_REGION_BITS)+((1<<SUB_REGION_BITS)-(1<<line_bit_number));
        unsigned int this_down_limitation=last_written_sub_region<<SUB_REGION_BITS;
        if(inner_crp[last_written_sub_region]==this_up_limitation)
        {
            //refresh_round++;
            inner_crp[last_written_sub_region]=this_down_limitation;
            inner_kp[last_written_sub_region]=inner_kc[last_written_sub_region];
            inner_kc[last_written_sub_region]=(rand()%pcm_size)<<line_bit_number;
            return true;
        }
        else
        {
            inner_crp[last_written_sub_region]=inner_crp[last_written_sub_region]+line_size;
        }
        new_cycle1=check_pointer_cycle(exchange_address>>line_bit_number,exchange_address>>line_bit_number,0);
        new_cycle2=check_pointer_cycle(inner_crp[last_written_sub_region]>>line_bit_number,inner_crp[last_written_sub_region]>>line_bit_number,0);
        new_target1=old_target2;
        new_target2=old_target1;
        if(old_cycle1||old_cycle2||new_cycle1||new_cycle2)
        {
            return true;
        }
        unsigned int remapped_address1,remapped_address2;
        bool remap_success1,remap_success2;
        if(pcm.lines[new_target1].write_count>=pcm.lines[new_target1].lifetime)//the block to be refreshed has worn out
        {
            remap_success1=remapping(new_target1,&remapped_address1);
            if(remap_success1)
            {
                new_target1=security_refresh_map(remapped_address1,true);
            }
            else
            {
                return false;
            }
        }
        if(pcm.lines[new_target2].write_count>=pcm.lines[new_target2].lifetime)//the block to be refreshed has worn out
        {
            remap_success2=remapping(new_target2,&remapped_address2);
            if(remap_success2)
            {
                new_target2=security_refresh_map(remapped_address2,true);
            }
            else
            {
                return false;
            }
        }

#ifdef POINTER_CACHE
        unsigned int source_address1;
        unsigned int source_address2;
        if(reverse_pointer_cache.lookup(&source_address1,new_target1))
        {
            pointer_cache.insert_entry(source_address1,new_target1);
            reverse_pointer_cache.insert_entry(source_address1,new_target1);
        }
        if(reverse_pointer_cache.lookup(&source_address2,new_target2))
        {
            pointer_cache.insert_entry(source_address2,new_target2);
            reverse_pointer_cache.insert_entry(source_address2,new_target2);
        }
#endif
        //exchange data
        unsigned int crp_line=inner_crp[last_written_sub_region]>>line_bit_number;
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
    total_access_delay+=50;
    //we do not need to consider read access.
    unsigned int mapped_address = wear_leveling_map(line_address,wl_method,true);
    //pointer_cache.invalid(mapped_address);
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
#ifdef PRINT_POINTER_DEPTH
            int percent=wear_out_count/(pcm_size*0.1);
            if((pointer_printed[percent]==false)&&(wear_out_count>0)&&((wear_out_count%(pcm_size/10)==0)))
            {
                pointer_printed[percent]=true;
                print_pointer();
            }
#endif
            return true;
            //wear_leveling("start_gap");
        }
    }
    else// dp == false , it is a pointer in that line.
    {
         return access_line(pcm.lines[mapped_address].remap_address,start_line_address,false,true,deepth+1);
    }
}
