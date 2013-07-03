#include "startgap.h"
#include "global.h"
#include <fstream>
#include <iostream>
using namespace std;
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
unsigned int region_count[1<<REGION_BITS];
unsigned int region_write_count[1<<REGION_BITS];
unsigned int region_gap[1<<REGION_BITS];
unsigned int region_start[1<<REGION_BITS];
unsigned int region_begin[1<<REGION_BITS];// first addresses of regions
unsigned int region_end[1<<REGION_BITS];// last addresses of regions
unsigned int gaps[1<<REGION_BITS];//extra lines for all gaps.
unsigned int region_visible_size=1<<(SUB_REGION_BITS-line_bit_number);
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

bool init_start_gap()//for non-regioned start-gap
{
    ifstream random_map_file(random);
    if(random_map_file.is_open()==false)
    {
        cout<<"Error: opening start-gap random file: "<<random<<endl;
        return false;
    }
    unsigned int i=0;
    while(random_map_file.eof()==0)
    {
        random_map_file>>random_map[i];
        i++;
    }
    if(random_map_file.is_open())
    {
        random_map_file.close();
    }
    return true;
}

bool init_region_start_gap()//for regioned start-gap
{
    ifstream random_map_file(random);
    if(random_map_file.is_open()==false)
    {
        cout<<"Error: opening start-gap random file: "<<random<<endl;
        return false;
    }
    unsigned int i=0;
    while(random_map_file.eof()==0)
    {
        random_map_file>>random_map[i];
        i++;
    }
    if(random_map_file.is_open())
    {
        random_map_file.close();
    }
    for(i=0;i<(1<<REGION_BITS);i++)
    {
        region_count[i]=0;
        region_begin[i]=i<<(SUB_REGION_BITS-line_bit_number);
        region_end[i]=region_begin[i]+(1<<(SUB_REGION_BITS-line_bit_number))-1;
        region_start[i]=0;
        region_gap[i]=0xffffffff;//0xffffffff means it is in extra space for lines.
    }
    return true;
}

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


bool region_start_gap()
{
#define PRE_WL
#ifdef PRE_WL
    if(last_written_line==0xffffffff)return true;//We ignore writes to extra line.
    unsigned int last_written_region=last_written_line>>(SUB_REGION_BITS-line_bit_number);
    unsigned int this_region_write_count=region_write_count[last_written_region]+gaps[last_written_region];
    if(this_region_write_count%frequency1==0)
    {
        //We do not need to do real wear_leveling because there is no actual data.
        if(region_gap[last_written_region]==region_begin[last_written_region])
        {
            region_start[last_written_region]=(region_start[last_written_region]+1)%region_visible_size;
            access_line(region_gap[last_written_region],region_gap[last_written_region],false,true,0);//[gap]=[n]    //real read and write occur! implemented in main.cpp
            region_gap[last_written_region]=0xffffffff;//gap is in extra lines.
        }
        else if(region_gap[last_written_region]==0xffffffff)// gap is moved from extra space to normal space.
        {
            //access_line(region_gap[last_written_region],region_gap[last_written_region],false,true,0);
            gaps[last_written_region]++;

            //when a line in extra space wears out, we consider it as device failure.
            if(gaps[last_written_region]>PCM_CELL_ENDURANCE)return false;
            region_gap[last_written_region]=region_end[last_written_region];
        }
        else
        {
            access_line(region_gap[last_written_region],region_gap[last_written_region],false,true,0); //[gap]=[gap-1]  //real read and write occur!
            region_gap[last_written_region]=region_gap[last_written_region]-1;
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

unsigned int region_start_gap_map(unsigned int line_address)
{
   unsigned int mapped_address=line_address;

#ifdef PRE_WL
    unsigned int this_region_number=line_address>>(SUB_REGION_BITS-line_bit_number);
    unsigned int inner_address=(line_address<<(32-(SUB_REGION_BITS-line_bit_number)))>>(32-(SUB_REGION_BITS-line_bit_number));
    inner_address=(inner_address+region_start[this_region_number])%region_visible_size;
    if((inner_address<<line_bit_number)<region_gap[this_region_number])
    {
        //remapped_address=region_begin[this_region_number]+(inner_address<<line_bit_number);
        return mapped_address;
    }
    else if(inner_address==((1<<(SUB_REGION_BITS-line_bit_number))-1))//
    {
        gaps[this_region_number]++;
        mapped_address=0xffffffff;//line_address is mapped to an extra line.
    }
    else
    {
        mapped_address=mapped_address+1;
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
   return mapped_address;
}
