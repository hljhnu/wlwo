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

unsigned int wear_leveling_map(unsigned int line_address,char* method)
{
 //unsigned int line_address = memory_address/line_size;
    unsigned int remapped_address=0;
    if(strcmp(method,"security_refresh")==0)
    {
        remapped_address=security_refresh_map(line_address);
    }
    else if(strcmp(method,"start_gap")==0)
    {
        remapped_address=start_gap_map(line_address);
    }
    return remapped_address;//it should return bool value. because address 0 is valid.
}

bool wear_leveling(char* method)
{
    if(strcmp(method,"security_refresh")==0)
    {
        return security_refresh();
    }
    else if(strcmp(method,"start_gap")==0)
    {
        return start_gap();
    }
}
