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
extern unsigned int region_count[];
extern unsigned int region_write_count[];
extern unsigned int region_gap[];
extern unsigned int region_start[];
extern unsigned int region_begin[];// first addresses of regions
extern unsigned int region_end[];// last addresses of regions
extern unsigned int gaps[];//extra lines for all gaps.
extern unsigned int region_visible_size;
extern bool init_start_gap();
extern bool start_gap();
extern unsigned int start_gap_map(unsigned int line_address);
extern bool init_region_start_gap();
extern bool region_start_gap();
extern unsigned int region_start_gap_map(unsigned int line_address);
#endif // _STARTGAP_H_
