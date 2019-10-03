/* ***********************
	Interface for python 
	- Swig. Copyright (c) 
	2003 Erica Andrews
	License: GNU General
	Public License (GPL)

*********************** */
int hw_initialized;
void do_init(void);
void do_uninit(void);
int get_hw_result_count(void);
int probe_hardware(char*);
char* get_search_result_line(int);





