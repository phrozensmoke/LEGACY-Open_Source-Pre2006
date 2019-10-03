#include <stdio.h>
#include <malloc.h>
#include <string.h>

FILE *my_dev_file;

char* my_held_data[1800];  // hold up to 1,800 lines of hw data
int printer;
int my_held_pointer;
void progArray(int);
void dump_sys(hd_data_t *, hd_t *, FILE *);

void reset_progArray(void);
void dump_cpu(hd_data_t *, hd_t *, FILE *);

void dump_normal(hd_data_t *, hd_t *, FILE *);


char* makeArray(int);
static int ind = 0;		/* output indentation */
#define dump_line(x0, x1...)  progArray(snprintf(  makeArray(  fprintf(my_dev_file, "%*s" x0, 0, "", x1)  ), 140,  "%*s" x0, 0, "", x1) ) 
#define dump_line_str(x0...) progArray(snprintf(  makeArray(  fprintf(my_dev_file, "%*s%s", 0, "", x0)  ), 140, "%*s%s", ind, "", x0) )
#define dump_line0(x0...) progArray(snprintf(  makeArray(  fprintf(my_dev_file, x0)  ), 140, x0) )
