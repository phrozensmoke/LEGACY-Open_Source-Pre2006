#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "hd.h"
#include "hdp.h"  
#include "hd_int.h"
#include "icewmcphw.h"


// ##### temporary solution, fix it later!
str_list_t *read_file(char *file_name, unsigned start_line, unsigned lines);
str_list_t *search_str_list(str_list_t *sl, char *str);
str_list_t *add_str_list(str_list_t **sl, char *str);
char *new_str(const char *);

static char *log_file = "";
static int is_short = 0;

static char *showconfig = NULL;
static char *saveconfig = NULL;
static int hw_item[100] = { };
static  int hw_items = 0;
int out_print=0;  //whether to print results to console or not
int options_length=72;

int pyproblems[]= {pr_sys, pr_max,  pr_lxrc    };  
 /*  these need to be probed before any Gtk X clients start, or the PyGtk program will 'disconnect' from X  */


int pyall[]={  pr_memory, pr_pci, pr_pci_range, pr_pci_ext, pr_isapnp, pr_cdrom,
  pr_cdrom_info, pr_net, pr_floppy, pr_misc, pr_misc_serial, pr_misc_par,
  pr_misc_floppy, pr_serial  , pr_cpu, pr_bios, pr_monitor,pr_ide,
  pr_scsi, pr_scsi_geo, pr_scsi_cache, pr_usb, pr_usb_mods, pr_adb,
  pr_modem, pr_modem_usb, pr_parallel, pr_parallel_lp, pr_parallel_zip,
  pr_isa, pr_isa_isdn, pr_dac960, pr_int, pr_manual , pr_mouse ,   pr_smart, pr_isdn, pr_kbd, pr_prom,
  pr_sbus, pr_braille, pr_braille_alva, pr_braille_fhp,
  pr_braille_ht  , pr_dasd  , pr_i2o, pr_cciss, pr_bios_vbe ,
  pr_isapnp_old , pr_ignx11  , pr_isapnp_new, pr_isapnp_mod , pr_braille_baum,
  pr_fb, pr_bios_vbe2, pr_veth , pr_partition  , pr_disk  , pr_ataraid    
  };

hd_data_t *hd_data;
char  *argy[2];
hd_t  *hd0=NULL;

/*  braille_install_info method was removed, PhrozenSmoke */

int x11_install_info(hd_data_t *hd_data);
int oem_install_info(hd_data_t *hd_data);
int dump_packages(hd_data_t *hd_data);

static void do_hw(hd_data_t *hd_data, FILE *f, hd_hw_item_t hw_item);
void do_short(hd_data_t *hd_data, hd_t *hd, FILE *f);
void do_test(hd_data_t *hd_data);
void help(void);
void dump_db_raw(hd_data_t *hd_data);
void dump_db(hd_data_t *hd_data);


struct {
  unsigned db_idx;
} opt;

struct option options[] = {
  { "--help", 1, NULL, 'h' },
  { "--debug", 1, NULL, 'd' },
  { "--log", 1, NULL, 'l' },
  { "--packages", 0, NULL, 'p' },
  { "--test", 0, NULL, 300 },
  { "--format", 1, NULL, 301 },
  { "--x11", 0, NULL, 7000 },
  { "--oem", 0, NULL, 7001 },
  { "--show-config", 1, NULL, 302 },
  { "--save-config", 1, NULL, 303 },
  { "--short", 0, NULL, 304 },
  { "--fast", 0, NULL, 305 },
  { "--dump-db", 1, NULL, 306 },
  { "--dump-db-raw", 1, NULL, 307 },
  { "--cdrom", 0, 0, 1000 + hw_cdrom },
  { "--floppy", 0, NULL, 1000 + hw_floppy },
  { "--disk", 0, NULL, 1000 + hw_disk },
  { "--network", 0, NULL, 1000 + hw_network },
  { "--display", 0, NULL, 1000 + hw_display },
  { "--gfxcard", 0, NULL, 1000 + hw_display },
  { "--framebuffer", 0, NULL, 1000 + hw_framebuffer },
  { "--monitor", 0, NULL, 1000 + hw_monitor },
  { "--camera", 0, NULL, 1000 + hw_camera },
  { "--mouse", 0, NULL, 1000 + hw_mouse },
  { "--joystick", 0, NULL, 1000 + hw_joystick },
  { "--keyboard", 0, NULL, 1000 + hw_keyboard },
  { "--chipcard", 0, NULL, 1000 + hw_chipcard },
  { "--sound", 0, NULL, 1000 + hw_sound },
  { "--isdn", 0, NULL, 1000 + hw_isdn },
  { "--modem", 0, NULL, 1000 + hw_modem },
  { "--storage-ctrl", 0, NULL, 1000 + hw_storage_ctrl },
  { "--storage_ctrl", 0, NULL, 1000 + hw_storage_ctrl },
  { "--netcard", 0, NULL, 1000 + hw_network_ctrl },
  { "--netcards", 0, NULL, 1000 + hw_network_ctrl },	// outdated, just kept for comaptibility
  { "--network-ctrl", 0, NULL, 1000 + hw_network_ctrl },
  { "--network_ctrl", 0, NULL, 1000 + hw_network_ctrl },
  { "--printer", 0, NULL, 1000 + hw_printer },
  { "--tv", 0, NULL, 1000 + hw_tv },
  { "--dvb", 0, NULL, 1000 + hw_dvb },
  { "--scanner", 0, NULL, 1000 + hw_scanner },
  { "--braille", 0, NULL, 1000 + hw_braille },
  { "--sys", 0, NULL, 1000 + hw_sys },
  { "--bios", 0, NULL, 1000 + hw_bios },
  { "--cpu", 0, NULL, 1000 + hw_cpu },
  { "--partition", 0, NULL, 1000 + hw_partition },
  { "--usb-ctrl", 0, NULL, 1000 + hw_usb_ctrl },
  { "--usb_ctrl", 0, NULL, 1000 + hw_usb_ctrl },
  { "--usb", 0, NULL, 1000 + hw_usb },
  { "--pci", 0, NULL, 1000 + hw_pci },
  { "--isapnp", 0, NULL, 1000 + hw_isapnp },
  { "--scsi", 0, NULL, 1000 + hw_scsi },
  { "--ide", 0, NULL, 1000 + hw_ide },
  { "--bridge", 0, NULL, 1000 + hw_bridge },
  { "--hub", 0, NULL, 1000 + hw_hub },
  { "--memory", 0, NULL, 1000 + hw_memory },
  { "--manual", 0, NULL, 1000 + hw_manual },
  { "--pcmcia", 0, NULL, 1000 + hw_pcmcia },
  { "--pcmcia-ctrl", 0, NULL, 1000 + hw_pcmcia_ctrl },
  { "--ieee1394", 0, NULL, 1000 + hw_ieee1394 },
  { "--ieee1394-ctrl", 0, NULL, 1000 + hw_ieee1394_ctrl },
  { "--firewire", 0, NULL, 1000 + hw_ieee1394 },
  { "--firewire-ctrl", 0, NULL, 1000 + hw_ieee1394_ctrl },
  { "--hotplug", 0, NULL, 1000 + hw_hotplug },
  { "--hotplug-ctrl", 0, NULL, 1000 + hw_hotplug_ctrl },
  { "--zip", 0, NULL, 1000 + hw_zip },
  { "--all", 0, NULL, 2000 },
  { "--reallyall", 0, NULL, 2001 },
  { "--smp", 0, NULL, 2002 },
  { "--arch", 0, NULL, 2003 },
  { "--system", 0, NULL, 6090 },
  { "--gtkall", 0, NULL, 5555},
  { "--gtkpre", 0, NULL, 5556},
  { "--cpuv", 0, NULL, 6091}
};



int get_option(char *arg_val)  {    
	int cc;
	struct option o;
   //printf("get_option  %s\n",arg_val);
   //fflush(stdout);
	for (cc=0; cc<options_length; cc++) {
		o=options[cc];
		//printf("o.name:  %s",o.name);
		//fflush(stdout);
		if (strstr(o.name,arg_val) != NULL) { return o.val;}
												  }
	return -1;

										} //end get_option



/*
 * Just scan the hardware and dump all info.
 */


int runHwCheck(int argc, char **argv)
{


  FILE *f = NULL;
  int i;
  int yc;
  int held_counter;
  
  reset_progArray();  //reset everything
  hw_items = 0;  //reset everything
  //my_held_data[0]="               ";

   
   //hd_data = malloc(sizeof (*hd_data));  
  hd_data->progress=NULL;
  //hd_data->progress = isatty(1) ? progress : NULL;
  hd_data->log=NULL;

  /* IMPORTANT: do not change the line below: will cause nasty crashes on probing 'pci' and 'isapnp' !!  */
  hd_data->debug=~(HD_DEB_DRIVER_INFO | HD_DEB_HDDB); 

  for(i = 0; i < argc; i++) {
   //printf("args: %s   %d\n",argv[i],argc);
   //fflush(stdout);


  if (strstr(argv[i], "--") != argv[i]) {continue;}
  else {
    /* new style interface */    
    yc=get_option(argv[i]);

   //printf("yc:   %d\n",yc);
   //fflush(stdout);

    if (yc  != -1) {
	//printf("switcher %d\n",yc);
	//fflush(stdout);
      switch(yc) {

        case 'd':
          hd_data->debug = strtol(optarg, NULL, 0);
          break;

        case 'l':
          log_file = optarg;
          break;

        case 'p':
          dump_packages(hd_data);
	  break;

        case 300:
          do_test(hd_data);
          break;

        case 301:
          hd_data->flags.dformat = strtol(optarg, NULL, 0);
          break;

        case 302:
          showconfig = optarg;
          break;

        case 303:
          saveconfig = optarg;
          break;

        case 304:
          is_short = 1;
          break;

        case 305:
          hd_data->flags.fast = 1;
          break;

        case 306:
          opt.db_idx = strtoul(optarg, NULL, 0);
          dump_db(hd_data);
          break;

        case 307:
          opt.db_idx = strtoul(optarg, NULL, 0);
          dump_db_raw(hd_data);
          break;

        case 7001:
		oem_install_info(hd_data);	
		break;

        case 1000 ... 1100:
          if(hw_items < sizeof hw_item / sizeof *hw_item)
            hw_item[hw_items++] = yc - 1000;
          break;

	case 7000:
            x11_install_info(hd_data);
	    break;

        case 5555:
        case 5556:
        case 2000:
        case 2001:
        case 2002:
        case 2003:
        case 6090:
        case 6091:
          if(hw_items < sizeof hw_item / sizeof *hw_item)
            hw_item[hw_items++] = yc;
          break;

        default:
          help();
          return 0;
      }  //end if


    }  //end else

  } //end for

    if(!hw_items && is_short) hw_item[hw_items++] = 2000;	/* all */

    if(hw_items >= 0 || showconfig || saveconfig) {
      if(*log_file) f = fopen(log_file, "w+");

      for(i = 0; i < hw_items; i++) {
        do_hw(hd_data, f, hw_item[i]);
      }



  /* for testing only */
  if (out_print==1) { //usually_commandline
  	held_counter=0;
  	printf("\n\nNumber of Lines:  %d\n\n",my_held_pointer);
  	while (held_counter<my_held_pointer) {
		printf("  %s",my_held_data[held_counter]); 
		if (strstr(my_held_data[held_counter],"\n")==NULL) {printf("\n");}
		held_counter++;
													}
	printf("\n\n");
	fflush(stdout);
				  } //end if


      if(f) fclose(f);
    }

    //hd_free_hd_data(hd_data);  // cant call this - causes seg-fault right now
    //free(hd_data);
	  //fclose(my_dev_file);
    return 0;
  }


    return 0;
}

void do_init(void) {
   int cc;
    if (my_held_pointer>0) {for (cc=0; cc<my_held_pointer ; cc++) { 

	if (my_held_data[cc] != NULL) {  my_held_data[cc]=NULL; }} 
							} 

   hd_data = calloc(1, sizeof *hd_data);  opterr = 0; my_dev_file=fopen("/dev/null","w"); hw_initialized=1;

				}


void do_uninit(void) {cfree(hd_data); hd_data=NULL; fclose(my_dev_file);  hw_initialized=0;}

int main(int argc, char **argv) {int cc; out_print=1; do_init(); cc=runHwCheck(argc,argv); do_uninit(); return cc;}

int get_hw_result_count(void) {return my_held_pointer;}

int probe_hardware(char* sel_hw) {
   char shw[strlen(sel_hw)+2];   
   if (hw_initialized==0) { do_init();  }
   sprintf(shw,"--%s",sel_hw);
   argy[0]="  ";
   argy[1]=shw;
   runHwCheck(2,argy);
   if (hw_initialized==1) { do_uninit(); }
   return get_hw_result_count();
											}


char* get_search_result_line(int myc) {
	if (myc>=my_held_pointer) {return NULL;}
        return my_held_data[myc];
										  }

void do_hw(hd_data_t *hd_data, FILE *f, hd_hw_item_t hw_item)
{
  hd_t *hd;

  int smp = -1, i;
  char *s, *t;
  enum boot_arch b_arch;
  enum cpu_arch c_arch;

  hd0 = NULL;
 
  if (out_print==1) {
   printf("\nProbing hardware item  %d  ...please wait...\n",hw_item);
   fflush(stdout);
				  }


  switch(hw_item) {

    case 2002:
      smp = hd_smp_support(hd_data);
      break;


    case 5555:
	hd_clear_probe_feature(hd_data, pr_all);	
	for (i=0; i<sizeof pyall / sizeof *pyall ; i++) {
		hd_set_probe_feature(hd_data, pyall[i]);
							  										 }
        hd_scan(hd_data);
	smp = hd_smp_support(hd_data);
        hd0 = hd_data->hd;
        break;

    case 5556:
	hd_clear_probe_feature(hd_data, pr_all);	
	for (i=0; i<sizeof pyproblems / sizeof *pyproblems ; i++) {
		hd_set_probe_feature(hd_data, pyproblems[i]);
							  										 }
        hd_scan(hd_data);
        hd0 = hd_data->hd;
        break;

    case 2000:
    case 2001:
    case 2003:
      i = -1;
      switch((int) hw_item) {
        case 2000: i = pr_default; break;
        case 2001: i = pr_all; break;
        case 2003: i = pr_cpu; break;
      }
      if(i != -1) {
        hd_clear_probe_feature(hd_data, pr_all);

        hd_set_probe_feature(hd_data, i);
        //hd_clear_probe_feature(hd_data, pr_mouse);
        hd_scan(hd_data);
        hd0 = hd_data->hd;
      }
      break;

    default:
	hd_set_probe_feature(hd_data, pr_cpu);
	//hd_scan(hd_data);
	 if (hd0==NULL) {  // called as command-line program, with one arg, no 'cache' needed
          hd0 = hd_list(hd_data, hw_item, 1, NULL); 
						}  else {hd0 = hd_list(hd_data, hw_item, 0, NULL); } // use hardware 'cache'
           //else {hd0 = hd_list(hd_data, hw_item, 0, hd_data->hd);}
  }


  if(f) {
    if((hd_data->debug & HD_DEB_SHOW_LOG) && hd_data->log) {
      fprintf(f,
        "============ start hardware log ============\n"
      );
      fprintf(f,
        "============ start debug info ============\n%s=========== end debug info ============\n",
        hd_data->log
      );
    }

    for(hd = hd_data->hd; hd; hd = hd->next) {
      hd_dump_entry(hd_data, hd, f);
    }

    fprintf(f,
      "============ end hardware log ============\n"
    );
  }

 if(hw_item == 2002 || hw_item==5555) {
    dump_line0("[HARDWARE-ENTRY]\n");
    dump_line0("Device: SMP\n");
    dump_line0("Hardware Class: SMP\n");
    dump_line("SMP support: %s",   smp < 0 ? "unknown" : smp > 0 ? "yes" : "no" );
    if(smp > 0) dump_line( " (%u cpus)\n", smp);
  }
  if(hw_item == 2003 || hw_item==5555) {
    c_arch = hd_cpu_arch(hd_data);
    b_arch = hd_boot_arch(hd_data);

    s = t = "Unknown";
    switch(c_arch) {
      case arch_unknown:
        break;
      case arch_intel:
        s = "X86 (32)";
        break;
      case arch_alpha:
        s = "Alpha";
        break;
      case arch_sparc:
        s = "Sparc (32)";
        break;
      case arch_sparc64:
        s = "UltraSparc (64)";
        break;
      case arch_ppc:
        s = "PowerPC";
        break;
      case arch_ppc64:
        s = "PowerPC (64)";
        break;
      case arch_68k:
        s = "68k";
        break;
      case arch_ia64:
        s = "IA-64";
        break;
      case arch_s390:
        s = "S390";
        break;
      case arch_s390x:
        s = "S390x";
        break;
      case arch_arm:
        s = "ARM";
        break;
      case arch_mips:
        s = "MIPS";
        break;
      case arch_x86_64:
        s = "X86_64";
        break;
    }

    switch(b_arch) {
      case boot_unknown:
        break;
      case boot_lilo:
        t = "lilo";
        break;
      case boot_milo:
        t = "milo";
        break;
      case boot_aboot:
        t = "aboot";
        break;
      case boot_silo:
        t = "silo";
        break;
      case boot_ppc:
        t = "ppc";
        break;
      case boot_elilo:
        t = "elilo";
        break;
      case boot_s390:
        t = "s390";
        break;
      case boot_mips:
        t = "mips";
        break;
      case boot_grub:
        t = "grub";
        break;
    }

    dump_line0("[HARDWARE-ENTRY]\n");
    dump_line0("Device: Architecture\n");
    dump_line0("Hardware Class: Architecture\n");
    dump_line( "Arch: %s/%s\n", s, t);
  }
  if (hw_item != 2002 && hw_item != 2003) {
    if(is_short) {
      /* always to stdout */
      do_short(hd_data, hd0, stdout);
      if(f) do_short(hd_data, hd0, f);
    }
    else     {


    if(hw_item == 6090) {
        hd_clear_probe_feature(hd_data, pr_all);
        hd_set_probe_feature(hd_data, pr_cpu);
        hd_scan(hd_data);
        hd0 = hd_list(hd_data,hw_cpu,0,NULL);
	dump_sys(hd_data,hd0, stdout);
						}
    else if(hw_item == 6091) {
        hd_clear_probe_feature(hd_data, pr_all);
        hd_set_probe_feature(hd_data, pr_cpu);
        hd_scan(hd_data);
        hd0 = hd_list(hd_data,hw_cpu,0,NULL);
	dump_cpu(hd_data,hd0, stdout);
						}
    else {

      for(hd = hd0; hd; hd = hd->next) {	
  	//printf("\n\nmy_held_data\n\n");
        //fflush(stdout);
	dump_line0("[HARDWARE-ENTRY]");
        hd_dump_entry(hd_data, hd, f ? f : stdout);
      											}

        hd0=hd_free_hd_list(hd0);

	if(hd0 != hd_data->hd) {  hd_free_hd_list(hd_data->hd);}


	   }
               }
  }

  if(hw_item == hw_display && hd0) {
    dump_line("Primary display adapter: #%u", hd_display_adapter(hd_data));
  }

  /* if(hd0 != hd_data->hd) {  //bug work-around for crash on 'bios' lookup
	 if (hw_item != 28) {hd_free_hd_list(hd0); } } */


}


void do_short(hd_data_t *hd_data, hd_t *hd, FILE *f)
{
#ifndef LIBHD_TINY
  hd_hw_item_t item;
  hd_t *hd1;
  int i;
  char *s;

  for(item = 1; item < hw_all; item++) {
    i = 0;
    s = hd_hw_item_name(item);
    if(!s) continue;

    if(item == hw_sys) continue;

    for(hd1 = hd; hd1; hd1 = hd1->next) {
      if(hd1->hw_class == item) {
        if(!i++) dump_line("%s:\n", s);
        dump_line("  %-20s %s\n",
          hd1->unix_dev_name ? hd1->unix_dev_name : "",
          hd1->model ? hd1->model : "???"
        );
      }
    }
  }
#endif
}


#if 1
typedef struct {
  char *vendor, *model, *driver;
} scanner_t;

static scanner_t scanner_data[] = {
  { "Abaton", "SCAN 300/GS", "abaton" },
  { "Abaton", "SCAN 300/S", "abaton" },
  { "Acer", "300f", "SnapScan" },
  { "Acer", "310s", "SnapScan" },
  { "Acer", "610plus", "SnapScan" },
  { "Acer", "610s", "SnapScan" },
  { "Acer", "Prisa 1240", "SnapScan" },
  { "Acer", "Prisa 3300", "SnapScan" },
  { "Acer", "Prisa 4300", "SnapScan" },
  { "Acer", "Prisa 5300", "SnapScan" },
  { "Acer", "Prisa 620s", "SnapScan" },
  { "Acer", "Prisa 620u", "SnapScan" },
  { "Acer", "Prisa 620ut", "SnapScan" },
  { "Acer", "Prisa 640bu", "SnapScan" },
  { "Acer", "Prisa 640u", "SnapScan" },
  { "Agfa", "Arcus II", "microtek" },
  { "Agfa", "DuoScan", "microtek" },
  { "Agfa", "FOCUS COLOR", "agfafocus" },
  { "Agfa", "FOCUS GS SCANNER", "agfafocus" },
  { "Agfa", "FOCUS II", "agfafocus" },
  { "Agfa", "FOCUS LINEART SCANNER", "agfafocus" },
  { "Agfa", "SnapScan 1212u", "SnapScan" },
  { "Agfa", "SnapScan 1236s", "SnapScan" },
  { "Agfa", "SnapScan 1236u", "SnapScan" },
  { "Agfa", "SnapScan 300", "SnapScan" },
  { "Agfa", "SnapScan 310", "SnapScan" },
  { "Agfa", "SnapScan 600", "SnapScan" },
  { "Agfa", "SnapScan e20", "SnapScan" },
  { "Agfa", "SnapScan e25", "SnapScan" },
  { "Agfa", "SnapScan e40", "SnapScan" },
  { "Agfa", "SnapScan e50", "SnapScan" },
  { "Agfa", "SnapScan e60", "SnapScan" },
  { "Agfa", "StudioScan", "microtek" },
  { "Agfa", "StudioScan II", "microtek" },
  { "Agfa", "StudioScan IIsi", "microtek" },
  { "Apple", "APPLE SCANNER", "apple" },
  { "Apple", "COLORONESCANNER", "apple" },
  { "Apple", "ONESCANNER", "apple" },
  { "Artec", "A6000C", "artec" },
  { "Artec", "A6000C PLUS", "artec" },
  { "Artec", "AM12S", "artec" },
  { "Artec", "AT12", "artec" },
  { "Artec", "AT3", "artec" },
  { "Artec", "AT6", "artec" },
  { "Artec", "ColorOneScanner", "artec" },
  { "Avision", "AV 620 CS", "avision" },
  { "Avision", "AV 6240", "avision" },
  { "Avision", "AV 630 CS", "avision" },
  { "B&H SCSI", "COPISCAN II 2135", "bh" },
  { "B&H SCSI", "COPISCAN II 2137", "bh" },
  { "B&H SCSI", "COPISCAN II 2137A", "bh" },
  { "B&H SCSI", "COPISCAN II 2138A", "bh" },
  { "B&H SCSI", "COPISCAN II 3238", "bh" },
  { "B&H SCSI", "COPISCAN II 3338", "bh" },
  { "B&H SCSI", "COPISCAN II 6338", "bh" },
  { "BlackWidow", "BW4800SP", "artec" },
  { "Canon", "CANOSCAN 2700F", "canon" },
  { "Canon", "CANOSCAN 300", "canon" },
  { "Canon", "CANOSCAN 600", "canon" },
  { "Devcom", "9636PRO", "pie" },
  { "Devcom", "9636S", "pie" },
  { "EDGE", "KTX-9600US", "umax" },
  { "Epson", "ES-8500", "epson" },
  { "Epson", "EXPRESSION 1600", "epson" },
  { "Epson", "EXPRESSION 1680", "epson" },
  { "Epson", "EXPRESSION 636", "epson" },
  { "Epson", "EXPRESSION 800", "epson" },
  { "Epson", "FILMSCAN 200", "epson" },
  { "Epson", "GT-5500", "epson" },
  { "Epson", "GT-7000", "epson" },
  { "Epson", "GT-8000", "epson" },
  { "Epson", "PERFECTION 1200PHOTO", "epson" },
  { "Epson", "PERFECTION 1200S", "epson" },
  { "Epson", "PERFECTION 1200U", "epson" },
  { "Epson", "PERFECTION 1240", "epson" },
  { "Epson", "PERFECTION 1640", "epson" },
  { "Epson", "PERFECTION 1650", "epson" },
  { "Epson", "PERFECTION 610", "epson" },
  { "Epson", "PERFECTION 636S", "epson" },
  { "Epson", "PERFECTION 636U", "epson" },
  { "Epson", "PERFECTION 640", "epson" },
  { "Epson", "PERFECTION1200", "epson" },
  { "Epson", "Perfection 600", "umax" },
  { "Escom", "Image Scanner 256", "umax" },
  { "Escort", "Galleria 600", "umax" },
  { "Fujitsu", "M3091DCD", "m3091" },
  { "Fujitsu", "M3096G", "m3096g" },
  { "Fujitsu", "SP15C", "sp15c" },
  { "Genius", "ColorPage-HR5 Pro", "umax" },
  { "Guillemot", "Maxi Scan A4 Deluxe", "SnapScan" },
  { "HP", "HP OFFICEJET K SERIES", "hp" },
  { "HP", "HP OFFICEJET V SERIES", "hp" },
  { "HP", "HP PHOTOSMART PHOTOSCANNER", "hp" },
  { "HP", "HP PSC 700 SERIES", "hp" },
  { "HP", "HP PSC 900 SERIES", "hp" },
  { "HP", "HP SCANJET 3C", "hp" },
  { "HP", "HP SCANJET 3P", "hp" },
  { "HP", "HP SCANJET 4100C", "hp" },
  { "HP", "HP SCANJET 4C", "hp" },
  { "HP", "HP SCANJET 4P", "hp" },
  { "HP", "HP SCANJET 5200C", "hp" },
  { "HP", "HP SCANJET 6100C", "hp" },
  { "HP", "HP SCANJET 6200C", "hp" },
  { "HP", "HP SCANJET 6250C", "hp" },
  { "HP", "HP SCANJET 6300C", "hp" },
  { "HP", "HP SCANJET 6350C", "hp" },
  { "HP", "HP SCANJET 6390C", "hp" },
  { "HP", "HP SCANJET IIC", "hp" },
  { "HP", "HP SCANJET IICX", "hp" },
  { "HP", "HP SCANJET IIP", "hp" },
  { "HP", "HP ScanJet 5p", "hp" },
  { "HP", "HP4200", "hp4200" },
  { "Highscreen", "Scanboostar Premium", "umax" },
  { "Linotype Hell", "Jade", "umax" },
  { "Linotype Hell", "Jade2", "umax" },
  { "Linotype Hell", "Linoscan 1400", "umax" },
  { "Linotype Hell", "Opal", "umax" },
  { "Linotype Hell", "Opal Ultra", "umax" },
  { "Linotype Hell", "Saphir", "umax" },
  { "Linotype Hell", "Saphir HiRes", "umax" },
  { "Linotype Hell", "Saphir Ultra", "umax" },
  { "Linotype Hell", "Saphir Ultra II", "umax" },
  { "Linotype Hell", "Saphir2", "umax" },
  { "Microtek", "Phantom 636", "microtek2" },
  { "Microtek", "ScanMaker 330", "microtek2" },
  { "Microtek", "ScanMaker 3600", "sm3600" },
  { "Microtek", "ScanMaker 630", "microtek2" },
  { "Microtek", "ScanMaker 636", "microtek2" },
  { "Microtek", "ScanMaker 9600XL", "microtek2" },
  { "Microtek", "ScanMaker E3plus", "microtek2" },
  { "Microtek", "ScanMaker V300", "microtek2" },
  { "Microtek", "ScanMaker V310", "microtek2" },
  { "Microtek", "ScanMaker V600", "microtek2" },
  { "Microtek", "ScanMaker V6USL", "microtek2" },
  { "Microtek", "ScanMaker X6", "microtek2" },
  { "Microtek", "ScanMaker X6EL", "microtek2" },
  { "Microtek", "ScanMaker X6USB", "microtek2" },
  { "Microtek", "Scanmaker 35", "microtek" },
  { "Microtek", "Scanmaker 35t+", "microtek" },
  { "Microtek", "Scanmaker 45t", "microtek" },
  { "Microtek", "Scanmaker 600G", "microtek" },
  { "Microtek", "Scanmaker 600G S", "microtek" },
  { "Microtek", "Scanmaker 600GS", "microtek" },
  { "Microtek", "Scanmaker 600S", "microtek" },
  { "Microtek", "Scanmaker 600Z", "microtek" },
  { "Microtek", "Scanmaker 600Z S", "microtek" },
  { "Microtek", "Scanmaker 600ZS", "microtek" },
  { "Microtek", "Scanmaker E2", "microtek" },
  { "Microtek", "Scanmaker E3", "microtek" },
  { "Microtek", "Scanmaker E6", "microtek" },
  { "Microtek", "Scanmaker II", "microtek" },
  { "Microtek", "Scanmaker IIG", "microtek" },
  { "Microtek", "Scanmaker IIHR", "microtek" },
  { "Microtek", "Scanmaker III", "microtek" },
  { "Microtek", "Scanmaker IISP", "microtek" },
  { "Microtek", "SlimScan C6", "microtek2" },
  { "Mustek", "1200 CU", "mustek_usb" },
  { "Mustek", "1200 CU Plus", "mustek_usb" },
  { "Mustek", "1200 UB", "mustek_usb" },
  { "Mustek", "600 CU", "mustek_usb" },
  { "Mustek", "Paragon 1200 A3 Pro", "mustek" },
  { "Mustek", "Paragon 1200 III SP", "mustek" },
  { "Mustek", "Paragon 1200 LS", "mustek" },
  { "Mustek", "Paragon 1200 SP Pro", "mustek" },
  { "Mustek", "Paragon 600 II CD", "mustek" },
  { "Mustek", "Paragon 800 II SP", "mustek" },
  { "Mustek", "Paragon MFC-600S", "mustek" },
  { "Mustek", "Paragon MFC-800S", "mustek" },
  { "Mustek", "Paragon MFS-12000CX", "mustek" },
  { "Mustek", "Paragon MFS-12000SP", "mustek" },
  { "Mustek", "Paragon MFS-1200SP", "mustek" },
  { "Mustek", "Paragon MFS-6000CX", "mustek" },
  { "Mustek", "Paragon MFS-6000SP", "mustek" },
  { "Mustek", "Paragon MFS-8000SP", "mustek" },
  { "Mustek", "ScanExpress 12000SP", "mustek" },
  { "Mustek", "ScanExpress 12000SP Plus", "mustek" },
  { "Mustek", "ScanExpress 6000SP", "mustek" },
  { "Mustek", "ScanExpress A3 SP", "mustek" },
  { "Mustek", "ScanMagic 600 II SP", "mustek" },
  { "Mustek", "ScanMagic 9636S", "mustek" },
  { "Mustek", "ScanMagic 9636S Plus", "mustek" },
  { "NEC", "PC-IN500/4C", "nec" },
  { "Nikon", "AX-210", "umax" },
  { "Nikon", "LS-1000", "coolscan" },
  { "Nikon", "LS-20", "coolscan" },
  { "Nikon", "LS-2000", "coolscan" },
  { "Nikon", "LS-30", "coolscan" },
  { "Pie", "9630S", "pie" },
  { "Pie", "ScanAce 1230S", "pie" },
  { "Pie", "ScanAce 1236S", "pie" },
  { "Pie", "ScanAce 630S", "pie" },
  { "Pie", "ScanAce 636S", "plustek" },
  { "Pie", "ScanAce II", "pie" },
  { "Pie", "ScanAce II Plus", "pie" },
  { "Pie", "ScanAce III", "pie" },
  { "Pie", "ScanAce III Plus", "pie" },
  { "Pie", "ScanAce Plus", "pie" },
  { "Pie", "ScanAce ScanMedia", "pie" },
  { "Pie", "ScanAce ScanMedia II", "pie" },
  { "Pie", "ScanAce V", "pie" },
  { "Plustek", "OpticPro 19200S", "artec" },
  { "Polaroid", "DMC", "dmc" },
  { "Ricoh", "Ricoh IS50", "ricoh" },
  { "Ricoh", "Ricoh IS60", "ricoh" },
  { "Scanport", "SQ4836", "microtek2" },
  { "Sharp", "9036 Flatbed scanner", "sharp" },
  { "Sharp", "JX-250", "sharp" },
  { "Sharp", "JX-320", "sharp" },
  { "Sharp", "JX-330", "sharp" },
  { "Sharp", "JX-350", "sharp" },
  { "Sharp", "JX-610", "sharp" },
  { "Siemens", "9036 Flatbed scanner", "s9036" },
  { "Siemens", "FOCUS COLOR PLUS", "agfafocus" },
  { "Siemens", "ST400", "st400" },
  { "Siemens", "ST800", "st400" },
  { "Tamarack", "Artiscan 12000C", "tamarack" },
  { "Tamarack", "Artiscan 6000C", "tamarack" },
  { "Tamarack", "Artiscan 8000C", "tamarack" },
  { "Trust", "Compact Scan USB 19200", "mustek_usb" },
  { "Trust", "Imagery 1200 SP", "mustek" },
  { "Trust", "Imagery 4800 SP", "mustek" },
  { "Trust", "SCSI Connect 19200", "mustek" },
  { "Trust", "SCSI excellence series 19200", "mustek" },
  { "UMAX", "Astra 1200S", "umax" },
  { "UMAX", "Astra 1220S", "umax" },
  { "UMAX", "Astra 2100S", "umax" },
  { "UMAX", "Astra 2200", "umax" },
  { "UMAX", "Astra 2200 S", "umax" },
  { "UMAX", "Astra 2200 U", "umax" },
  { "UMAX", "Astra 2400S", "umax" },
  { "UMAX", "Astra 600S", "umax" },
  { "UMAX", "Astra 610S", "umax" },
  { "UMAX", "Gemini D-16", "umax" },
  { "UMAX", "Mirage D-16L", "umax" },
  { "UMAX", "Mirage II", "umax" },
  { "UMAX", "Mirage IIse", "umax" },
  { "UMAX", "PL-II", "umax" },
  { "UMAX", "PSD", "umax" },
  { "UMAX", "PowerLook", "umax" },
  { "UMAX", "PowerLook 2000", "umax" },
  { "UMAX", "PowerLook 3000", "umax" },
  { "UMAX", "PowerLook III", "umax" },
  { "UMAX", "Supervista S-12", "umax" },
  { "UMAX", "UC 1200S", "umax" },
  { "UMAX", "UC 1200SE", "umax" },
  { "UMAX", "UC 1260", "umax" },
  { "UMAX", "UC 630", "umax" },
  { "UMAX", "UC 840", "umax" },
  { "UMAX", "UG 630", "umax" },
  { "UMAX", "UG 80", "umax" },
  { "UMAX", "UMAX S-12", "umax" },
  { "UMAX", "UMAX S-12G", "umax" },
  { "UMAX", "UMAX S-6E", "umax" },
  { "UMAX", "UMAX S-6EG", "umax" },
  { "UMAX", "UMAX VT600", "umax" },
  { "UMAX", "Vista S6", "umax" },
  { "UMAX", "Vista S6E", "umax" },
  { "UMAX", "Vista-S8", "umax" },
  { "UMAX", "Vista-T630", "umax" },
  { "Ultima", "A6000C", "artec" },
  { "Ultima", "A6000C PLUS", "artec" },
  { "Ultima", "AM12S", "artec" },
  { "Ultima", "AT12", "artec" },
  { "Ultima", "AT3", "artec" },
  { "Ultima", "AT6", "artec" },
  { "Ultima", "ColorOneScanner", "artec" },
  { "Vobis", "HighScan", "microtek2" },
  { "Vobis", "Scanboostar Premium", "umax" },
  { "Vuego", "Close SnapScan 310 compatible.", "SnapScan" }
};

static char *scanner_info(hd_t *hd)
{
  int i;

  if(!hd->vendor.name || !hd->device.name) return NULL;

  for(i = 0; i < sizeof scanner_data / sizeof *scanner_data; i++) {
    if(
      !strcasecmp(scanner_data[i].vendor, hd->vendor.name) &&
      !strcasecmp(scanner_data[i].model, hd->device.name)
    ) {
      return scanner_data[i].driver;
    }
  }

  return NULL;
}

#endif

void do_test(hd_data_t *hd_data)
{
#if 1
  hd_t *hd, *hd0;
  hd_res_t *res;
  driver_info_t *di;
  FILE *f;
  int i, wheels, buttons;
  unsigned u;
  uint64_t ul;
  char *s, *s1;
  hd_hw_item_t item, items[] = {
    hw_display, hw_monitor, hw_tv, hw_sound, hw_mouse, hw_disk, hw_cdrom,
    hw_floppy, hw_modem, hw_isdn, hw_scanner, hw_camera
  };

  hd_set_probe_feature(hd_data, pr_default);
  hd_scan(hd_data);

  f = fopen("/tmp/hw_overview.log", "w");

  for(i = 0; i < sizeof items / sizeof *items; i++) {
    item = items[i];
    hd0 = hd_list(hd_data, item, 0, NULL);

    if(!hd0) continue;
  
    switch(item) {
      case hw_disk:
        fprintf(f, "Disk\n");
        for(hd = hd0; hd; hd = hd->next) {
          u = 0;
          for(res = hd->res; res; res = res->next) {
            if(res->any.type == res_size && res->size.unit == size_unit_sectors) {
              ul = (uint64_t) res->size.val1 * (res->size.val2 ?: 0x200);
              u = ((ul >> 29) + 1) >> 1;
            }
          }
          s = hd->bus.name;
          fprintf(f, "  %s", hd->model);
          if(u) {
            fprintf(f, " (");
            if(s) fprintf(f, "%s, ", s);
            fprintf(f, "%u GB)", u);
          }
          fprintf(f, "\n");
        }
        fprintf(f, "\n");
        break;

      case hw_cdrom:
        fprintf(f, "CD-ROM\n");
        for(hd = hd0; hd; hd = hd->next) {
          s = hd->bus.name;
          fprintf(f, "  %s (", hd->model);
          if(s) fprintf(f, "%s, ", s);
          fprintf(f, "%s)", hd->prog_if.name ?: "CD-ROM");
          fprintf(f, "\n");
        }
        fprintf(f, "\n");
        break;

      case hw_monitor:
        fprintf(f, "Monitor\n");
        for(hd = hd0; hd; hd = hd->next) {
          s = hd->model;
          if(!strcmp(hd->unique_id, "rdCR.EY_qmtb9YY0")) s = "not detected";
          fprintf(f, "  %s\n", s);
        }
        fprintf(f, "\n");
        break;

      case hw_display:
        fprintf(f, "GFX Card\n");
        for(hd = hd0; hd; hd = hd->next) {
          u = 0;
          s1 = NULL;
          for(di = hd->driver_info; di; di = di->next) {
            if(di->any.type == di_x11) {
              if(!s1) s1 = di->x11.server;
              if(di->x11.x3d && !u) {
                s1 = di->x11.server;
                u = 1;
              }
            }
          }
          if(!s1) {
            s1 = "not supported";
            u = 0;
          }
          fprintf(f, "  %s (%s", hd->model, s1);
          if(u) fprintf(f, ", 3D support");
          fprintf(f, ")");
          fprintf(f, "\n");
        }
        fprintf(f, "\n");
        break;

      case hw_mouse:
        fprintf(f, "Mouse\n");
        for(hd = hd0; hd; hd = hd->next) {
          buttons = wheels = -1;	// make gcc happy
          s = NULL;
          for(di = hd->driver_info; di; di = di->next) {
            if(di->any.type == di_mouse) {
              buttons = di->mouse.buttons;
              wheels = di->mouse.wheels;
              s = di->mouse.xf86;
              break;
            }
          }
          if(!s) {
            s = "not supported";
            buttons = wheels = -1;
          }
          fprintf(f, "  %s (%s", hd->model, s);
	  //progArray(sprintf(my_held_data[my_held_pointer],"  %s (%s", hd->model, s));
          if(buttons >= 0) fprintf(f, ", %d buttons", buttons);
          if(wheels >= 0) fprintf(f, ", %d wheels", wheels);
          fprintf(f, ")");
          fprintf(f, "\n");
        }
        fprintf(f, "\n");
        break;

      case hw_tv:
        fprintf(f, "TV Card\n");
        for(hd = hd0; hd; hd = hd->next) {
          s = NULL;
          for(di = hd->driver_info; di; di = di->next) {
            if(
              (di->any.type == di_any || di->any.type == di_module) &&
              di->any.hddb0 &&
              di->any.hddb0->str
            ) {
              s = di->any.hddb0->str;
              break;
            }
          }
          if(!s) {
            s = "not supported";
          }
          fprintf(f, "  %s (%s)\n", hd->model, s);
        }
        fprintf(f, "\n");
        break;

      case hw_sound:
        fprintf(f, "Sound Card\n");
        for(hd = hd0; hd; hd = hd->next) {
          s = NULL;
          for(di = hd->driver_info; di; di = di->next) {
            if(
              (di->any.type == di_any || di->any.type == di_module) &&
              di->any.hddb0 &&
              di->any.hddb0->str
            ) {
              s = di->any.hddb0->str;
              break;
            }
          }
          if(!s) {
            s = "not supported";
          }
          fprintf(f, "  %s (%s)\n", hd->model, s);
        }
        fprintf(f, "\n");
        break;

      case hw_camera:
        fprintf(f, "Digital Camera/WebCam\n");
        for(hd = hd0; hd; hd = hd->next) {
          fprintf(f, "  %s\n", hd->model);
        }
        fprintf(f, "\n");
        break;

      case hw_floppy:
        fprintf(f, "Floppy/Zip Drive\n");
        for(hd = hd0; hd; hd = hd->next) {
          fprintf(f, "  %s\n", hd->model);
        }
        fprintf(f, "\n");
        break;

      case hw_modem:
        fprintf(f, "Modem\n");
        for(hd = hd0; hd; hd = hd->next) {
          fprintf(f, "  %s\n", hd->model);
        }
        fprintf(f, "\n");
        break;

      case hw_isdn:
        fprintf(f, "ISDN\n");
        for(hd = hd0; hd; hd = hd->next) {
          fprintf(f, "  %s (%ssupported)\n", hd->model, hd->driver_info ? "" : "not ");
        }
        fprintf(f, "\n");
        break;

      case hw_scanner:
        fprintf(f, "Scanner\n");
        for(hd = hd0; hd; hd = hd->next) {
          s = scanner_info(hd);
          if(!s) s = "not supported";
          fprintf(f, "  %s (%s)\n", hd->model, s);
        }
        fprintf(f, "\n");
        break;

      default:
        break;
    }

    //hd_free_hd_list(hd0);

  }

  fclose(f);

  f = fopen("/tmp/hw_detail.log", "w");

  if(hd_data->log) {
    fprintf(f,
      "============ start detailed hardware log ============\n"
    );
    fprintf(f,
      "============ start debug info ============\n%s=========== end debug info ============\n",
      hd_data->log
    );
  }

  for(hd = hd_data->hd; hd; hd = hd->next) {
    hd_dump_entry(hd_data, hd, f);
  }

  fprintf(f,
    "============ end detailed hardware log ============\n"
  );

  fclose(f);

  fprintf(stderr, "\n");

#endif

#if 0
  hd_t *hd;
  hd_t *hd0 = NULL;

  for(hd = hd_list(hd_data, hw_cdrom, 1, hd0); hd; hd = hd->next) {
    fprintf(stderr, "cdrom: %s, %s\n", hd->unix_dev_name, hd->model);
  }

  for(hd = hd_list(hd_data, hw_cdrom, 1, hd0); hd; hd = hd->next) {
    fprintf(stderr, "cdrom: %s, %s\n", hd->unix_dev_name, hd->model);
  }
#endif

#if 0
  hd_t *hd;

  hd = hd_list(hd_data, hw_disk, 1, NULL);
  //hd_free_hd_list(hd);
  hd_free_hd_data(hd_data);

  hd = hd_list(hd_data, hw_cdrom, 1, NULL);
  ////hd_free_hd_list(hd);
  hd_free_hd_data(hd_data);

  hd = hd_list(hd_data, hw_storage_ctrl, 1, NULL);
  //hd_free_hd_list(hd);
  hd_free_hd_data(hd_data);

  hd = hd_list(hd_data, hw_display, 1, NULL);
  //hd_free_hd_list(hd);
  hd_free_hd_data(hd_data);

#if 0
  for(hd = hd_data->hd; hd; hd = hd->next) {
    hd_dump_entry(hd_data, hd, stdout);
  }

  printf("%s\n", hd_data->log);
#endif

#endif

#if 0
  hd_t *hd, *hd0;

  hd0 = hd_list(hd_data, hw_sound, 1, NULL);
  hd0 = hd_list(hd_data, hw_sound, 1, NULL);

  for(hd = hd0; hd; hd = hd->next) {
    hd_dump_entry(hd_data, hd, stdout);
  }

#if 0
  hd_data->log = free_mem(hd_data->log);
  dump_hddb_data(hd_data, hd_data->hddb_dev, "hddb_dev, final");  
  if(hd_data->log) printf("%s", hd_data->log);
#endif

#endif
}


void help()
{
  fprintf(stdout,
    "\nhwinfo version 5.39  [a modified versin of SuSe's 'hwinfo' tool]\n\n"
    "Usage: hwinfo2 [options]\n"
    "Probe for hardware.\n"
    "  --short        just a short listing\n"
    "  --log logfile  write info to logfile\n"
    "  --debug level  set debuglevel\n"
    "  --dump-db n    dump hardware data base, 0: external, 1: internal\n"
    "  --hw_item      probe for hw_item\n"
    "  hw_item is one of:\n"
    "    cdrom, floppy, disk, network, gfxcard, framebuffer, monitor, camera,\n"
    "    mouse, joystick, keyboard, chipcard, sound, isdn, modem, storage-ctrl,\n"
    "    netcard, printer, tv, dvb, scanner, braille, sys, bios, cpu, partition,\n"
    "    usb-ctrl, usb, pci, isapnp, ide, scsi, bridge, hub, memory, smp,\n"
    "    zip, hotplug, hotplug-ctrl, pcmcia-ctrl, pcmcia, firewire, firewire-ctrl,\n"
    "    x11, oem, display, ieee1394, ieee1394-ctrl, arch, all, reallyall, test\n\n"
    "  Note: debug info is shown only in the log file. (If you specify a\n"
    "  log file the debug level is implicitly set to a reasonable value.)\n\n"
  );
}




#define INSTALL_INF	"/etc/install.inf"

/*
 * get VGA parameter from /proc/cmdline
 */
int get_fb_mode()
{
#ifndef __PPC__
  FILE *f;
  char buf[256], *s, *t;
  int i, fb_mode = 0;

  if((f = fopen("/proc/cmdline", "r"))) {
    if(fgets(buf, sizeof buf, f)) {
      t = buf;
      while((s = strsep(&t, " "))) {
        if(sscanf(s, "vga=%i", &i) == 1) fb_mode = i;
        if(strstr(s, "vga=normal") == s) fb_mode = 0;
      }
    }
    fclose(f);
  }

  return fb_mode > 0x10 ? fb_mode : 0;
#else /* __PPC__ */
  /* this is the only valid test for active framebuffer ... */
  FILE *f = NULL;
  int fb_mode = 0;
  if((f = fopen("/dev/fb", "r"))) {
    fb_mode++;
    fclose(f);
  }

  return fb_mode;
#endif
}


/*
 * read "x11i=" entry from /proc/cmdline
 */
char *get_x11i()
{
  FILE *f;
  char buf[256], *s, *t;
  static char x11i[64] = { };

  if(!*x11i) return x11i;

  if((f = fopen("/proc/cmdline", "r"))) {
    if(fgets(buf, sizeof buf, f)) {
      t = buf;
      while((s = strsep(&t, " "))) {
        if(sscanf(s, "x11i=%60s", x11i) == 1) break;
      }
    }
    fclose(f);
  }

  return x11i;
}


/*
 * Assumes xf86_ver to be either "3" or "4" (or empty).
 */
char *get_xserver(hd_data_t *hd_data, char **version, char **busid, driver_info_t **x11_driver)
{
  static char display[16];
  static char xf86_ver[2];
  static char id[32];
  char c, *x11i = get_x11i();
  driver_info_t *di;
  hd_t *hd;

  *x11_driver = NULL;

  *display = *xf86_ver = *id = c = 0;
  *version = xf86_ver;
  *busid = id;

  if(x11i) {
    if(*x11i == '3' || *x11i == '4') {
      c = *x11i;
    }
    else {
      if(*x11i >= 'A' && *x11i <= 'Z') {
        c = '3';
      }
      if(*x11i >= 'a' && *x11i <= 'z') {
        c = '4';
      }
      if(c) {
        strncpy(display, x11i, sizeof display - 1);
        display[sizeof display - 1] = 0;
      }
    }
  }

  if(c) { xf86_ver[0] = c; xf86_ver[1] = 0; }

  hd = hd_get_device_by_idx(hd_data, hd_display_adapter(hd_data));

  if(hd && hd->bus.id == bus_pci)
    sprintf(id, "%d:%d:%d", hd->slot >> 8, hd->slot & 0xff, hd->func);

  if(!hd || *display) return display;

  for(di = hd->driver_info; di; di = di->next) {
    if(di->any.type == di_x11 && di->x11.server && di->x11.xf86_ver && !di->x11.x3d) {
      if(c == 0 || c == di->x11.xf86_ver[0]) {
        xf86_ver[0] = di->x11.xf86_ver[0];
        xf86_ver[1] = 0;
        strncpy(display, di->x11.server, sizeof display - 1);
        display[sizeof display - 1] = 0;
        *x11_driver = di;
        break;
      }
    }
  }


  if(*display) return display;

  if(c == 0) c = '4';	/* default to XF 4, if nothing else is known  */

  xf86_ver[0] = c;
  xf86_ver[1] = 0;
  strcpy(display, c == '3' ? "FBDev" : "fbdev");

  return display;
}

int x11_install_info(hd_data_t *hd_data)
{
  hd_t *hd;
  driver_info_t *di;
  char *x11i;
  int fb_mode, kbd_ok = 0;
  unsigned yast2_color = 0;
  char *xkbrules = NULL, *xkbmodel = NULL, *xkblayout = NULL;
  char *xserver, *version, *busid;
  driver_info_t *x11_driver;
  str_list_t *sl;

  /* NOTE: The X11 display scan is currently non-fuctional (seg-faulting), will be fixed in the future - 
     return a 'dummy' hardware entry for now   */
    dump_line0("[HARDWARE-ENTRY]\n");
    dump_line0("Device: X Server  [sorry, X Server probing not yet implemented]\n");
    dump_line0("Hardware Class: unknown\n");
    return 1;

  hd_set_probe_feature(hd_data, pr_cpu);
  hd_set_probe_feature(hd_data, pr_prom);
  hd_scan(hd_data);
  //version="4";

  x11i = get_x11i();
  fb_mode = get_fb_mode();

  printf("here 1\n");
fflush(stdout);

  hd_list(hd_data, hw_display, 1, NULL);

  printf("here 1\n");
fflush(stdout);

    dump_line0("[HARDWARE-ENTRY]");
    dump_line0("Device: X Server");
    dump_line0("Hardware Class: unknown");

  for(hd = hd_list(hd_data, hw_keyboard, 0, NULL); hd; hd = hd->next) {
    kbd_ok = 1;
    di = hd->driver_info;
    if(di && di->any.type == di_kbd) {
      xkbrules = di->kbd.XkbRules;
      xkbmodel = di->kbd.XkbModel;
      xkblayout = di->kbd.XkbLayout;
      break;
    }
    /* don't free di */
  }
  printf("here 1\n");
fflush(stdout);
  xserver = get_xserver(hd_data, &version, &busid, &x11_driver);

  switch(hd_mac_color(hd_data)) {
    case 0x01:
      yast2_color = 0x5a4add;
      break;
    case 0x04:
      yast2_color = 0x32cd32;
      break;
    case 0x05:
      yast2_color = 0xff7f50;
      break;
    case 0x07:
      yast2_color = 0x000000;
      break;
    case 0xff:
      yast2_color = 0x7f7f7f;
      break;
  }

  printf("here 4\n");
fflush(stdout); 
  dump_line( "Keyboard: %d\n", kbd_ok);
  printf("here 4a\n");
fflush(stdout); 
  if(fb_mode) dump_line( "Framebuffer: 0x%04x\n", fb_mode);
  printf("here 4b\n");
fflush(stdout); 
  //if(x11i) dump_line( "X11i: %s\n", x11i);
  if(xserver && *xserver) {
  printf("here 4b-2\n");
fflush(stdout); 
    //dump_line( "XServer: %s\n", xserver);
  printf("here 4c\n");
fflush(stdout); 
    if(*version) dump_line( "XVersion: %s\n", version);
  printf("here 4d\n");
fflush(stdout); 
    if(*busid) dump_line( "XBusID: %s\n", busid);
  printf("here 4e\n");
fflush(stdout); 
  }


 /*  disabled below due to seg-faulting - PhrozenSMoke */
 /* if(xkbrules && *xkbrules) dump_line( "XkbRules: %s\n", xkbrules);
  if(xkbmodel && *xkbmodel) dump_line( "XkbModel: %s\n", xkbmodel);
  if(xkblayout && *xkblayout) dump_line( "XkbLayout: %s\n", xkblayout); */

  printf("here 6\n");
fflush(stdout); 
  if(x11_driver) {
    for(sl = x11_driver->x11.extensions; sl; sl = sl->next) {
      if(*sl->str) dump_line( "XF86Ext:   Load\t\t\"%s\"\n", sl->str);
    }
    for(sl = x11_driver->x11.options; sl; sl = sl->next) {
      if(*sl->str) dump_line( "XF86Raw:   Option\t\"%s\"\n", sl->str);
    }
    for(sl = x11_driver->x11.raw; sl; sl = sl->next) {
      if(*sl->str) dump_line( "XF86Raw:   %s\n", sl->str);
    }
  }
  return 0;
}

char *xserver3map[] =
{
#ifdef __i386__
  "VGA16", "xvga16",
  "RUSH", "xrush",
#endif
#if defined(__i386__) || defined(__alpha__) || defined(__ia64__)
  "SVGA", "xsvga",
  "3DLABS", "xglint",
#endif
#if defined(__i386__) || defined(__alpha__)
  "MACH64", "xmach64",
  "P9000", "xp9k",
  "S3", "xs3",
#endif
#ifdef __alpha__
  "TGA", "xtga",
#endif
#ifdef __sparc__
  "SUNMONO", "xsunmono",
  "SUN", "xsun",
  "SUN24", "xsun24",
#endif
#if 0
  "VMWARE", "xvmware",
#endif
  0, 0
};


int dump_packages(hd_data_t *hd_data)
{
  str_list_t *sl;
  int i;

  hd_data->progress = NULL;
  hd_scan(hd_data);

  sl = get_hddb_packages(hd_data);

  for(i = 0; xserver3map[i]; i += 2) {
    if (!search_str_list(sl, xserver3map[i + 1]))
      add_str_list(&sl, new_str(xserver3map[i + 1]));
  }

    dump_line0("[HARDWARE-ENTRY]\n");
    dump_line0("Device: X Server Packages\n");
    dump_line0("Hardware Class: unknown\n");

  for(; sl; sl = sl->next) {
    dump_line("Package: %s\n", sl->str);
  }

  return 0;
}

struct x11pack {
  struct x11pack *next;
  char *pack;
};

int oem_install_info(hd_data_t *hd_data)
{
  hd_t *hd;
  str_list_t *str;
  str_list_t *x11packs = 0;
  str_list_t *sl;
  FILE *f;

  int pcmcia, i;

  driver_info_x11_t *di, *drvinfo;

  hd_set_probe_feature(hd_data, pr_pci);
  hd_scan(hd_data);
  f=stdout;
    dump_line0("[HARDWARE-ENTRY]\n");
    dump_line0("Device: OEM\n");
    dump_line0("Hardware Class: unknown\n");
  pcmcia = hd_has_pcmcia(hd_data);
  for(hd = hd_list(hd_data, hw_display, 1, NULL); hd; hd = hd->next) {
    for(str = hd->requires; str; str = str->next) {
      if(!search_str_list(x11packs, str->str)) {
        add_str_list(&x11packs, str->str);
      }
    }
    drvinfo = (driver_info_x11_t *) hd->driver_info;
    for (di = drvinfo; di; di = (driver_info_x11_t *)di->next) {
      if (di->type != di_x11) {
	continue;  }
		
	        dump_normal(hd_data,hd,stdout);
		//return 0;
      if (di->xf86_ver[0] == '3') {

        char *server = di->server;

        if (server) {
	  for (i = 0; xserver3map[i]; i += 2)
	    if (!strcmp(xserver3map[i], server))
	      break;
	  if (xserver3map[i])
	    if (!search_str_list(x11packs, xserver3map[i + 1]))
	      add_str_list(&x11packs, xserver3map[i + 1]);
	}
      }
    }
  }

  if (x11packs) {
    //dump_line0("X11Packages: ");
    for (sl = x11packs; sl; sl = sl->next) {
      if (sl != x11packs)
       // fputc(',', f);
      dump_line("X11Packages: %s", sl->str);
    }
    //fputc('\n', f);
  }
  if (pcmcia)
    dump_line("Pcmcia: %d\n", pcmcia);
  //fclose(f);
  return 0;
}


void dump_db_raw(hd_data_t *hd_data)
{
  hd_data->progress = NULL;
  hd_clear_probe_feature(hd_data, pr_all);
  hd_scan(hd_data);

  if(opt.db_idx >= sizeof hd_data->hddb2 / sizeof *hd_data->hddb2) return;

  hddb_dump_raw(hd_data->hddb2[opt.db_idx], stdout);
}


void dump_db(hd_data_t *hd_data)
{
  hd_data->progress = NULL;
  hd_clear_probe_feature(hd_data, pr_all);
  hd_scan(hd_data);

  if(opt.db_idx >= sizeof hd_data->hddb2 / sizeof *hd_data->hddb2) return;

  hddb_dump(hd_data->hddb2[opt.db_idx], stdout);
}

