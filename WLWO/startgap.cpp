#include "startgap.h"
#include "global.h"
unsigned int visible_size=pcm_size-1;
unsigned int start1=0;
#ifdef PRE_WL
unsigned int gap1=pcm_size-1;
#else
unsigned int gap1=pcm_size-1;
#endif // PRE_WL
unsigned int frequency1=100;
//Number "1" at the end of some variables is for case
//when extra space are not used for wear-leveing before the first wear-out.

unsigned int start2=0;
unsigned int gap2=pcm_size-1;//two lines are used as gaps.
unsigned int frequency2=100;
unsigned int random_map[0xffffff];//-16384
//normal space : 0-->pivot
//back space : pivot+1-->pcm_size-1
//all space : 0-->pcm_size-1
//useful space : 0-->pcm_size-3
//the biggest address is pcm_size-1
/**

|-------normal space-------------------------|-----back space-----|
 *   *    *    *     *     *    *    *    *    *    *    *    *
 0                                      pivot              pcm_size-1

*/
bool start_gap()
{
#ifdef PRE_WL
    if(total_write_count%frequency1==0)
    {
        //We do not need to do real wear_leveing because there is no actual data.
        if(gap1==0)
        {
            start1=(start1+1)%visible_size;
            access_line(gap1,gap1,false,true,0);//[gap]=[n]    //real read and write occur! implemented in main.cpp
            gap1=pcm_size-1;
        }
        else
        {
            access_line(gap1,gap1,false,true,0); //[gap]=[gap-1]  //real read and write occur!
            gap1--;
        }
    }
#else
    if(total_write_count%frequency1==0)
    {
        //We do not need to do real wear_leveing because there is no actual data.
        if(gap1==0)
        {
            start1=(start1+1)%pivot;
            if(!access_line(gap1,gap1,false,true,0))return false;//[gap]=[n]    //real read and write occur! implemented in main.cpp
            gap1=pivot;
        }
        else
        {
            if(!access_line(gap1,gap1,false,true,0))return false; //[gap]=[gap-1]  //real read and write occur!
            gap1--;
        }
    }

    if(total_write_count2%frequency2==0)
    {
        //We do not need to do real wear_leveing because there is no actual data.
        if(gap2==pivot+1)//[pivot] is used for normal space rather than back space
        {
            start2=(start2-pivot+1)%(pcm_size-1-pivot-1)+pivot;
            access_line(gap2,gap2,false,true,0); //[gap]=[n]
            gap2=pcm_size-1;
        }
        else
        {
            access_line(gap2,gap2,false,true,0); //[gap]=[gap-1]
            gap2--;
        }
    }
#endif // PRE_WL
    return true;
}


unsigned int start_gap_map(unsigned int line_address)
{
   unsigned int remapped_address=line_address;

#ifdef PRE_WL
    remapped_address = (line_address+start1)%visible_size;
    if(remapped_address>=gap1)
    {
        remapped_address=remapped_address+1;
    }

#else
   if(line_address<pivot)
   {
       remapped_address = (line_address+start1)%(pivot-1);
       if(remapped_address>=gap1)
       {
           remapped_address=remapped_address+1;
       }
   }
   else
   {
       unsigned int temp_address = line_address-pivot;
       temp_address=(temp_address+start2)%(pcm_size-pivot-1)+pivot;
       if(temp_address>=gap2)
       {
           temp_address=temp_address+1;
       }
       remapped_address=temp_address;
   }
#endif // PRE_WL
   return remapped_address;
}
