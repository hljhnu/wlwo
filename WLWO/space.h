/**************************************************************************
This file is mainly used for modeling a PCM device. It simulates properties
as lifetime of cells.
***************************************************************************/

#ifndef _SPACE_H_
#define _SPACE_H_
#include <cstdlib>
#include "global.h"

/*
class byte
{
    unsigned char value;
    unsigned long bit_life_time[8];
    unsigned long bit_write_count[8];
};
*/
class line
{
    public:
    bool dpflag;//whether it is a data or pointer
    bool alive;//whether a line is wear out.
    unsigned int lifetime;
    unsigned int point_deep;//in order to save memory, we use unsigned char
    //class bytes[line_size];
    unsigned int remap_address;
    unsigned int write_count;
    line()
    {
        dpflag = true;//true:data
        alive = true;
        remap_address=0;
        write_count=0;
        lifetime=10000;
        point_deep=0;
    };
    //failure_bits[8*line_size];
};
/*
class page
{
    bool used;
    class line lines[page_size];
};
class bank
{

};
*/
class space
{
    public:
    unsigned int size;//total address
    unsigned int capacity;//valid address space
    unsigned int begin_address;
    class line lines[pcm_size];

    space(unsigned int size=pcm_size)
    {
        this->size=size;
        this->capacity=size;
        this->begin_address=pivot+1;
    };
};
/*
class back_space: space
{
    public:
    unsigned int begin_address ;
};
*/
extern class space pcm;
#endif
