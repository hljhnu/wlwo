#ifndef _STARTGAP_H_
#define _STARTGAP_H_
//#include "global.h"
extern unsigned int start1;
extern unsigned int gap1;//
extern unsigned int visible_size;
extern unsigned int frequency1;
extern unsigned int start2;
extern unsigned int gap2;//two lines are used as gaps.
extern unsigned int frequency2;
extern unsigned int random_map[];

extern bool init_start_gap();
extern bool start_gap();
extern unsigned int start_gap_map(unsigned int line_address);

#endif // _STARTGAP_H_
