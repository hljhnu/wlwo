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
    unsigned int i = 0;
    for(i=pcm.begin_address;i<pcm.size;i++)
    {
        if(pcm.lines[i].dpflag)
        {
            pcm.begin_address=i;
            break;
        }
    }
    if(pcm.size <= pcm.begin_address)//?
    {
        cout<<"Allocation failure: There is no space in backup device\n";
        return false;
    }
    *remapped_address=pcm.begin_address;//begin_address is a logical address
    unsigned int wl_pysical_address=wear_leveling_map(*remapped_address,wl_method);
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
    pcm.lines[line_address].write_count++;
#ifdef PRE_WL
    total_write_count++;
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
