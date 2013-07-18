#include "global.h"
#include "startgap.h"
#include "securityrefresh.h"
#include <cstring>
using namespace std;
/*******************************************************************
This function is used for wear-leveling map (security refresh). We only
simulate write accesses because read accesses do not have impact on
the lifetime of a pcm device.
in:
        memory_address:This is the address from the last level cache.
return :
        The returned value is the memory address after wear-leveling.
        If it is a fault block, it will be remapped to a valid block.
********************************************************************/

unsigned int wear_leveling_map(unsigned int line_address,char* method,bool update)
{
 //unsigned int line_address = memory_address/line_size;
    unsigned int mapped_address=line_address;
    if(strcmp(method,"security_refresh")==0)
    {
        mapped_address=security_refresh_map(line_address,update);
    }
    else if(strcmp(method,"start_gap")==0)
    {
        mapped_address=region_start_gap_map(line_address);
    }
    return mapped_address;//it should return bool value. because address 0 is valid.
}

bool wear_leveling(char* method)
{
    if(strcmp(method,"security_refresh")==0)
    {
        return security_refresh();
    }
    else if(strcmp(method,"start_gap")==0)
    {
        return region_start_gap();
    }
    return true;
}
