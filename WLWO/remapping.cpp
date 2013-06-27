#include "global.h"
#include "space.h"
#include <iostream>
using namespace std;
/******************************************************************************************
This function is used for remapping a fault block to a valid block.

in:
        physical_address: This is the physical address after wear-leveling,but before being
                       remapped to a valid block.
return:
        The returned value is the valid physical memory address in the extra address space.
*******************************************************************************************/
bool remapping(unsigned int physical_address,unsigned int *remapped_address)
{
    bool successful=true;
    successful=allocate_address(physical_address,remapped_address);
    if(successful)
    {
        pcm.lines[physical_address].dpflag=false;//the line on physical_address stores a pointer now.
        pcm.lines[physical_address].remap_address=*remapped_address;
        //cout<<"physical_address="<<physical_address<<" remapped_address="<<*remapped_address<<endl;
    }
    return successful;
}



bool allocate_address(unsigned int physical_address,unsigned int * remapped_address)
{
    if(pcm.size <= pcm.begin_address)//?
    {
        cout<<"Allocation failure: There is no space in backup device\n";
        return false;
    }
    *remapped_address=pcm.begin_address;//begin_address is a logical address
    unsigned int wl_pysical_address=wear_leveling_map(*remapped_address,wl_method,false);
    //pcm.lines[pcm.begin_address].write_count=1;
    pcm.lines[wl_pysical_address].point_deep=pcm.lines[physical_address].point_deep+1;
/*
    if(pcm.lines[wl_pysical_address].point_deep>1)
    {
        cout<<"deep: "<<pcm.lines[wl_pysical_address].point_deep<<endl;
    }
*/
    pcm.begin_address++;
    pcm.capacity--;
    return true;
}

void perform_access_pcm(unsigned int line_address)
{
    //pcm.lines[line_address].write_count+=(pcm.lines[line_address].lifetime>1);
    pcm.lines[line_address].write_count++;
    //last_written_line=line_address;
    //unsigned int written_sub_region=line_address>>(SUB_REGION_BITS-line_bit_number);//if the system does wear-leveling, this region will be chosen.
#ifdef PRE_WL
    //total_write_count++;
#else
    if(line_address<=pivot)
    {
        total_write_count++;
    }
    else
    {
        total_write_count2++;
    }
#endif // PRE_WL
}

unsigned int lookup_target(unsigned int line_address)//update:whether to update pointer deepth
{
    unsigned int mapped_address = wear_leveling_map(line_address,wl_method,false);
    if(pcm.lines[mapped_address].dpflag)//if dpflag == true , it is data in that cacheline.
    {
        return mapped_address;
    }
    else// dp == false , it is a pointer in that line.
    {
        unsigned int remapped_address=pcm.lines[mapped_address].remap_address;
        return lookup_target(remapped_address);
    }
}

bool check_pointer_cycle(unsigned int line_address, unsigned int start_line_address,unsigned int depth)//update:whether to update pointer deepth
{
    if((depth!=0)&&(line_address==start_line_address))
    {
        return true;
    }
    depth++;
    unsigned int mapped_address = wear_leveling_map(line_address,wl_method,false);
    if(pcm.lines[mapped_address].dpflag)//if dpflag == true , it is data in that cacheline.
    {
        return false;
    }
    else// dp == false , it is a pointer in that line.
    {
        unsigned int remapped_address=pcm.lines[mapped_address].remap_address;
        return check_pointer_cycle(remapped_address,start_line_address,depth);
    }
}
