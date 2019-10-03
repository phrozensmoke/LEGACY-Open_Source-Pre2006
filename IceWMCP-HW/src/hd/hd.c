#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <linux/pci.h>
#include <linux/hdreg.h>

#ifndef BLKSSZGET
#define BLKSSZGET _IO(0x12,104)		/* get block device sector size */
#endif

#include "hd.h"
#include "hddb.h"
#include "hd_int.h"
#include "memory.h"
#include "isapnp.h"
#include "monitor.h"
#include "pci.h"
#include "cpu.h"
#include "misc.h"
#include "mouse.h"
#include "floppy.h"
#include "ide.h"
#include "scsi.h"
#include "cdrom.h"
#include "bios.h"
#include "serial.h"
#include "net.h"
#include "version.h"
#include "usb.h"
#include "adb.h"
#include "modem.h"
#include "parallel.h"
#include "isa.h"
#include "dac960.h"
#include "smart.h"
#include "isdn.h"
#include "kbd.h"
#include "prom.h"
#include "sbus.h"
#include "int.h"
#include "braille.h"
#include "sys.h"
#include "dasd.h"
#include "i2o.h"
#include "cciss.h"
#include "manual.h"
#include "fb.h"
#include "veth.h"
#include "partition.h"
#include "disk.h"
#include "ataraid.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * various functions commmon to all probing modules
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

#ifdef __i386__
#define HD_ARCH "ia32"
#endif

#ifdef __ia64__
#define HD_ARCH "ia64"
#endif

#ifdef __alpha__
#define HD_ARCH "axp"
#endif

#ifdef __PPC__
#define HD_ARCH "ppc"
#endif

#ifdef __sparc__
#define HD_ARCH "sparc"
#endif

#ifdef __s390x__
#define HD_ARCH "s390x"
#else
#ifdef __s390__
#define HD_ARCH "s390"
#endif
#endif

#ifdef __arm__
#define HD_ARCH "arm"
#endif

#ifdef __mips__
#define HD_ARCH "mips"
#endif

#ifdef __x86_64__
#define HD_ARCH "x86-64"
#endif

typedef struct disk_s {
  struct disk_s *next;
  unsigned crc;
  unsigned crc_match:1;
  unsigned hd_idx;
  char *dev_name;
  unsigned char *data;
} disk_t;

static struct s_pr_flags *get_pr_flags(enum probe_feature feature);
static void fix_probe_features(hd_data_t *hd_data);
static void set_probe_feature(hd_data_t *hd_data, enum probe_feature feature, unsigned val);
static void free_old_hd_entries(hd_data_t *hd_data);
static hd_t *free_hd_entry(hd_t *hd);
static hd_t *add_hd_entry2(hd_t **hd, hd_t *new_hd);
static void timeout_alarm_handler(int signal);
static void get_probe_env(hd_data_t *hd_data);
static void hd_scan_xtra(hd_data_t *hd_data);

static void test_read_block0_open(void *arg);
static void get_kernel_version(hd_data_t *hd_data);
static void assign_hw_class(hd_data_t *hd_data, hd_t *hd);
#ifndef LIBHD_TINY
static void short_vendor(char *vendor);
static void create_model_name(hd_data_t *hd_data, hd_t *hd);
#endif

static int is_pcmcia_ctrl(hd_data_t *hd_data, hd_t *hd);


/*
 * Names of the probing modules.
 * Cf. enum mod_idx in hd_int.h.
 */
static struct s_mod_names {
  unsigned val;
  char *name;
} pr_modules[] = {
  { mod_none, "none"},
  { mod_memory, "memory"},
  { mod_pci, "pci"},
  { mod_isapnp, "isapnp"},
  { mod_pnpdump, "pnpdump"},
  { mod_cdrom, "cdrom"},
  { mod_net, "net"},
  { mod_floppy, "floppy"},
  { mod_misc, "misc" },
  { mod_bios, "bios"},
  { mod_cpu, "cpu"},
  { mod_monitor, "monitor"},
  { mod_serial, "serial"},
  { mod_mouse, "mouse"},
  { mod_ide, "ide"},
  { mod_scsi, "scsi"},
  { mod_usb, "usb"},
  { mod_adb, "adb"},
  { mod_modem, "modem"},
  { mod_parallel, "parallel" },
  { mod_isa, "isa" },
  { mod_dac960, "dac960" },
  { mod_smart, "smart" },
  { mod_isdn, "isdn" },
  { mod_kbd, "kbd" },
  { mod_prom, "prom" },
  { mod_sbus, "sbus" },
  { mod_int, "int" },
  { mod_braille, "braille" },
  { mod_xtra, "hd" },
  { mod_sys, "sys" },
  { mod_dasd, "dasd" },
  { mod_i2o, "i2o" },
  { mod_cciss, "cciss" },
  { mod_manual, "manual" },
  { mod_fb, "fb" },
  { mod_veth, "veth" },
  { mod_partition, "partition" },
  { mod_disk, "disk" },
  { mod_ataraid, "ataraid" }
};

/*
 * Names for the probe flags. Used for debugging and command line parsing in
 * hw.c. Cf. enum probe_feature, hd_data_t.probe.
 */
static struct s_pr_flags {
  enum probe_feature val, parent;
  unsigned mask;	/* bit 0: default, bit 1: all, bit 2: max, bit 3: linuxrc */
  char *name;
} pr_flags[] = {
  { pr_default,     -1,                 1, "default"      },
  { pr_all,         -1,               2  , "all"          },
  { pr_max,         -1,             4    , "max"          },
  { pr_lxrc,        -1,           8      , "lxrc"         },
  { pr_memory,       0,           8|4|2|1, "memory"       },
  { pr_pci,          0,           8|4|2|1, "pci"          },
  { pr_pci_range,    pr_pci,        4|2  , "pci.range"    },
  { pr_pci_ext,      pr_pci,        4|2  , "pci.ext"      },
  { pr_isapnp,       0,             4|2|1, "isapnp"       },
  { pr_isapnp_old,   pr_isapnp,         0, "isapnp.old"   },
  { pr_isapnp_new,   pr_isapnp,         0, "isapnp.new"   },
  { pr_isapnp_mod,   0,             4    , "isapnp.mod"   },
  { pr_isapnp,       0,                 0, "pnpdump"      },	/* alias for isapnp */
  { pr_cdrom,        0,           8|4|2|1, "cdrom"        },
  { pr_cdrom_info,   pr_cdrom,    8|4|2|1, "cdrom.info"   },
  { pr_net,          0,           8|4|2|1, "net"          },
  { pr_floppy,       0,           8|4|2|1, "floppy"       },
  { pr_misc,         pr_bios,     8|4|2|1, "misc"         },	// ugly hack!
  { pr_misc_serial,  pr_misc,     8|4|2|1, "misc.serial"  },
  { pr_misc_par,     pr_misc,       4|2|1, "misc.par"     },
  { pr_misc_floppy,  pr_misc,     8|4|2|1, "misc.floppy"  },
  { pr_bios,         0,           8|4|2|1, "bios"         },
  { pr_bios_vbe,     pr_bios,       4|2|1, "bios.vbe"     },
  { pr_bios_vbe2,    pr_bios,           0, "bios.vbe2"    },
  { pr_cpu,          0,           8|4|2|1, "cpu"          },
  { pr_monitor,      0,           8|4|2|1, "monitor"      },
  { pr_serial,       0,             4|2|1, "serial"       },
#if defined(__sparc__)
  /* Probe for mouse on SPARC */
  { pr_mouse,        0,           8|4|2|1, "mouse"        },
#else
  { pr_mouse,        0,             4|2|1, "mouse"        },
#endif
  { pr_ide,          0,           8|4|2|1, "ide"          },
  { pr_scsi,         0,           8|4|2|1, "scsi"         },
  { pr_scsi_geo,     pr_scsi,       4|2  , "scsi.geo"     },
  { pr_scsi_cache,   pr_scsi,       4|2|1, "scsi.cache"   },
  { pr_usb,          0,           8|4|2|1, "usb"          },
  { pr_usb_mods,     0,             4    , "usb.mods"     },
  { pr_adb,          0,           8|4|2|1, "adb"          },
  { pr_modem,        0,             4|2|1, "modem"        },
  { pr_modem_usb,    pr_modem,      4|2|1, "modem.usb"    },
  { pr_parallel,     0,             4|2|1, "parallel"     },
  { pr_parallel_lp,  pr_parallel,   4|2|1, "parallel.lp"  },
  { pr_parallel_zip, pr_parallel,   4|2|1, "parallel.zip" },
  { pr_isa,          0,             4|2|1, "isa"          },
  { pr_isa_isdn,     pr_isa,        4|2|1, "isa.isdn"     },
  { pr_dac960,       0,           8|4|2|1, "dac960"       },
  { pr_smart,        0,           8|4|2|1, "smart"        },
  { pr_isdn,         0,             4|2|1, "isdn"         },
  { pr_kbd,          0,           8|4|2|1, "kbd"          },
  { pr_prom,         0,           8|4|2|1, "prom"         },
  { pr_sbus,         0,           8|4|2|1, "sbus"         },
  { pr_int,          0,           8|4|2|1, "int"          },
#if defined(__i386__) || defined (__x86_64__)
  { pr_braille,      0,             4|2|1, "braille"      },
  { pr_braille_alva, pr_braille,    4|2|1, "braille.alva" },
  { pr_braille_fhp,  pr_braille,    4|2|1, "braille.fhp"  },
  { pr_braille_ht,   pr_braille,    4|2|1, "braille.ht"   },
  { pr_braille_baum, pr_braille,    4|2|1, "braille.baum" },
#else
  { pr_braille,      0,             4|2  , "braille"      },
  { pr_braille_alva, pr_braille,        0, "braille.alva" },
  { pr_braille_fhp,  pr_braille,    4|2  , "braille.fhp"  },
  { pr_braille_ht,   pr_braille,    4|2  , "braille.ht"   },
  { pr_braille_baum, pr_braille,    4|2  , "braille.baum" },
#endif
  { pr_ignx11,       0,                 0, "ignx11"       },
  { pr_sys,          0,           8|4|2|1, "sys"          },
  { pr_dasd,         0,           8|4|2|1, "dasd"         },
  { pr_i2o,          0,           8|4|2|1, "i2o"          },
  { pr_cciss,        0,           8|4|2|1, "cciss"        },
  { pr_manual,       0,           8|4|2|1, "manual"       },
  { pr_fb,           0,           8|4|2|1, "fb"           },
  { pr_veth,         0,           8|4|2|1, "veth"         },
  { pr_partition,    0,           8|4|2|1, "partition"    },
  { pr_disk,         0,           8|4|2|1, "disk"         },
  { pr_ataraid,      0,           8|4|2|1, "ataraid"      }
};

struct s_pr_flags *get_pr_flags(enum probe_feature feature)
{
  int i;

  for(i = 0; i < sizeof pr_flags / sizeof *pr_flags; i++) {
    if(feature == pr_flags[i].val) return pr_flags + i;
  }

  return NULL;
}

void fix_probe_features(hd_data_t *hd_data)
{
  int i;

  for(i = 0; i < sizeof hd_data->probe; i++) {
    hd_data->probe[i] |= hd_data->probe_set[i];
    hd_data->probe[i] &= ~hd_data->probe_clr[i];
  }
}

void set_probe_feature(hd_data_t *hd_data, enum probe_feature feature, unsigned val)
{
  unsigned ofs, bit, mask;
  int i;
  struct s_pr_flags *pr;

  if(!(pr = get_pr_flags(feature))) return;

  if(pr->parent == -1) {
    mask = pr->mask;
    for(i = 0; i < sizeof pr_flags / sizeof *pr_flags; i++) {
      if(pr_flags[i].parent != -1 && (pr_flags[i].mask & mask))
        set_probe_feature(hd_data, pr_flags[i].val, val);
    }
  }
  else {
    ofs = feature >> 3; bit = feature & 7;
    if(ofs < sizeof hd_data->probe) {
      if(val) {
        hd_data->probe_set[ofs] |= 1 << bit;
        hd_data->probe_clr[ofs] &= ~(1 << bit);
      }
      else {
        hd_data->probe_clr[ofs] |= 1 << bit;
        hd_data->probe_set[ofs] &= ~(1 << bit);
      }
    }
    if(pr->parent) set_probe_feature(hd_data, pr->parent, val);
  }

  fix_probe_features(hd_data);
}

void hd_set_probe_feature(hd_data_t *hd_data, enum probe_feature feature)
{
  unsigned ofs, bit, mask;
  int i;
  struct s_pr_flags *pr;

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(hd_set_probe_feature, hd_data), hd_data);
  }
#endif

  if(!(pr = get_pr_flags(feature))) return;

  if(pr->parent == -1) {
    mask = pr->mask;
    for(i = 0; i < sizeof pr_flags / sizeof *pr_flags; i++) {
      if(pr_flags[i].parent != -1 && (pr_flags[i].mask & mask))
        hd_set_probe_feature(hd_data, pr_flags[i].val);
    }
  }
  else {
    ofs = feature >> 3; bit = feature & 7;
    if(ofs < sizeof hd_data->probe)
      hd_data->probe[ofs] |= 1 << bit;
    if(pr->parent) hd_set_probe_feature(hd_data, pr->parent);
  }

  fix_probe_features(hd_data);
}

void hd_clear_probe_feature(hd_data_t *hd_data, enum probe_feature feature)
{
  unsigned ofs, bit, mask;
  int i;
  struct s_pr_flags *pr;

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(hd_clear_probe_feature, hd_data), hd_data);
  }
#endif

  if(!(pr = get_pr_flags(feature))) return;

  if(pr->parent == -1) {
    mask = pr->mask;
    for(i = 0; i < sizeof pr_flags / sizeof *pr_flags; i++) {
      if(pr_flags[i].parent != -1 && (pr_flags[i].mask & mask))
        hd_clear_probe_feature(hd_data, pr_flags[i].val);
    }
  }
  else {
    ofs = feature >> 3; bit = feature & 7;
    if(ofs < sizeof hd_data->probe)
      hd_data->probe[ofs] &= ~(1 << bit);
  }
}

int hd_probe_feature(hd_data_t *hd_data, enum probe_feature feature)
{
#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(hd_probe_feature, hd_data), hd_data);
  }
#endif

  if(feature < 0 || feature >= pr_default) return 0;

  return hd_data->probe[feature >> 3] & (1 << (feature & 7)) ? 1 : 0;
}


void hd_set_probe_feature_hw(hd_data_t *hd_data, hd_hw_item_t item)
{
  hd_set_probe_feature(hd_data, pr_int);
  hd_set_probe_feature(hd_data, pr_manual);

  switch(item) {
    case hw_cdrom:
      hd_set_probe_feature(hd_data, pr_pci);
      hd_set_probe_feature(hd_data, pr_usb);
      hd_set_probe_feature(hd_data, pr_ide);
      hd_set_probe_feature(hd_data, pr_scsi_cache);
      hd_set_probe_feature(hd_data, pr_cdrom_info);
      hd_set_probe_feature(hd_data, pr_partition);
      break;

    case hw_floppy:
      hd_set_probe_feature(hd_data, pr_floppy);
      hd_set_probe_feature(hd_data, pr_ide);
      hd_set_probe_feature(hd_data, pr_scsi_cache);	// really necessary?
      hd_set_probe_feature(hd_data, pr_misc_floppy);
      hd_set_probe_feature(hd_data, pr_prom);
      hd_set_probe_feature(hd_data, pr_pci);
      hd_set_probe_feature(hd_data, pr_usb);
      hd_set_probe_feature(hd_data, pr_partition);
      break;

    case hw_disk:
      hd_set_probe_feature(hd_data, pr_pci);
      hd_set_probe_feature(hd_data, pr_ide);
      hd_set_probe_feature(hd_data, pr_scsi_cache);
//      hd_set_probe_feature(hd_data, pr_scsi_geo);
      hd_set_probe_feature(hd_data, pr_dac960);
      hd_set_probe_feature(hd_data, pr_smart);
//      hd_set_probe_feature(hd_data, pr_i2o);
//      hd_set_probe_feature(hd_data, pr_cciss);
      hd_set_probe_feature(hd_data, pr_dasd);
      hd_set_probe_feature(hd_data, pr_partition);
      hd_set_probe_feature(hd_data, pr_disk);
      hd_set_probe_feature(hd_data, pr_ataraid);
      break;

    case hw_network:
      hd_set_probe_feature(hd_data, pr_net);
      break;

    case hw_display:
      hd_set_probe_feature(hd_data, pr_pci);
      hd_set_probe_feature(hd_data, pr_sbus);
      hd_set_probe_feature(hd_data, pr_prom);
      hd_set_probe_feature(hd_data, pr_misc);		/* for isa cards */
      break;

    case hw_monitor:
      hd_set_probe_feature(hd_data, pr_misc);
      hd_set_probe_feature(hd_data, pr_prom);
      hd_set_probe_feature(hd_data, pr_pci);
      hd_set_probe_feature(hd_data, pr_bios_vbe);
      hd_set_probe_feature(hd_data, pr_fb);
      hd_set_probe_feature(hd_data, pr_monitor);
      break;

    case hw_framebuffer:
      hd_set_probe_feature(hd_data, pr_misc);
      hd_set_probe_feature(hd_data, pr_prom);
      hd_set_probe_feature(hd_data, pr_pci);
      hd_set_probe_feature(hd_data, pr_bios_vbe);
      hd_set_probe_feature(hd_data, pr_fb);
      break;

    case hw_mouse:
      hd_set_probe_feature(hd_data, pr_misc);
      if(!hd_data->flags.fast) {
        hd_set_probe_feature(hd_data, pr_serial);
      }
      hd_set_probe_feature(hd_data, pr_adb);
      hd_set_probe_feature(hd_data, pr_usb);
      hd_set_probe_feature(hd_data, pr_kbd);
      hd_set_probe_feature(hd_data, pr_sys);
      hd_set_probe_feature(hd_data, pr_bios);
      hd_set_probe_feature(hd_data, pr_mouse);
      break;

    case hw_joystick:
      hd_set_probe_feature(hd_data, pr_usb);
      break;

    case hw_chipcard:
      hd_set_probe_feature(hd_data, pr_misc);
      if(!hd_data->flags.fast) {
        hd_set_probe_feature(hd_data, pr_serial);
      }
      hd_set_probe_feature(hd_data, pr_usb);
      hd_set_probe_feature(hd_data, pr_mouse);		/* we need the pnp code */
      break;

    case hw_camera:
      hd_set_probe_feature(hd_data, pr_usb);
      break;

    case hw_keyboard:
      hd_set_probe_feature(hd_data, pr_cpu);
      hd_set_probe_feature(hd_data, pr_misc);
      hd_set_probe_feature(hd_data, pr_adb);
      hd_set_probe_feature(hd_data, pr_usb);
      hd_set_probe_feature(hd_data, pr_kbd);
#ifdef __PPC__
      hd_set_probe_feature(hd_data, pr_serial);
#endif
      break;

    case hw_sound:
      hd_set_probe_feature(hd_data, pr_misc);
      hd_set_probe_feature(hd_data, pr_pci);
      hd_set_probe_feature(hd_data, pr_isapnp);
      hd_set_probe_feature(hd_data, pr_isapnp_mod);
      hd_set_probe_feature(hd_data, pr_sbus);
#ifdef __PPC__
      hd_set_probe_feature(hd_data, pr_prom);
#endif
      break;

    case hw_isdn:
      hd_set_probe_feature(hd_data, pr_misc);		/* get basic i/o res */
      hd_set_probe_feature(hd_data, pr_pci);
      hd_set_probe_feature(hd_data, pr_isapnp);
      hd_set_probe_feature(hd_data, pr_isapnp_mod);
      hd_set_probe_feature(hd_data, pr_isa_isdn);
      hd_set_probe_feature(hd_data, pr_usb);
      hd_set_probe_feature(hd_data, pr_isdn);
      break;

    case hw_modem:
      hd_set_probe_feature(hd_data, pr_misc);
      hd_set_probe_feature(hd_data, pr_serial);
      hd_set_probe_feature(hd_data, pr_usb);
      hd_set_probe_feature(hd_data, pr_modem);
      hd_set_probe_feature(hd_data, pr_modem_usb);
      break;

    case hw_storage_ctrl:
      hd_set_probe_feature(hd_data, pr_floppy);
      hd_set_probe_feature(hd_data, pr_sys);
      hd_set_probe_feature(hd_data, pr_pci);
      hd_set_probe_feature(hd_data, pr_sbus);
      if(!hd_data->flags.fast) {
        hd_set_probe_feature(hd_data, pr_misc_par);
        hd_set_probe_feature(hd_data, pr_parallel_zip);
      }
      hd_set_probe_feature(hd_data, pr_dasd);		/* dasd on s390 */
#ifdef __PPC__
      hd_set_probe_feature(hd_data, pr_prom);
      hd_set_probe_feature(hd_data, pr_misc);
      hd_set_probe_feature(hd_data, pr_ide);		/* dasd on iseries */
#endif
      break;

    case hw_network_ctrl:
      hd_set_probe_feature(hd_data, pr_misc);
      hd_set_probe_feature(hd_data, pr_pci);
      hd_set_probe_feature(hd_data, pr_isapnp);
      hd_set_probe_feature(hd_data, pr_isapnp_mod);
      hd_set_probe_feature(hd_data, pr_sbus);
      hd_set_probe_feature(hd_data, pr_isdn);
#ifdef __PPC__
      hd_set_probe_feature(hd_data, pr_prom);
#endif
#if defined(__s390__) || defined(__s390x__)
      hd_set_probe_feature(hd_data, pr_net);
#endif
      hd_set_probe_feature(hd_data, pr_veth);
      break;

    case hw_printer:
      hd_set_probe_feature(hd_data, pr_sys);
      hd_set_probe_feature(hd_data, pr_bios);
      hd_set_probe_feature(hd_data, pr_misc_par);
      hd_set_probe_feature(hd_data, pr_parallel_lp);
      hd_set_probe_feature(hd_data, pr_usb);
      break;

    case hw_tv:
    case hw_dvb:
      hd_set_probe_feature(hd_data, pr_pci);
      break;

    case hw_scanner:
      hd_set_probe_feature(hd_data, pr_usb); 
      hd_set_probe_feature(hd_data, pr_scsi); 
      hd_set_probe_feature(hd_data, pr_partition);
      break;

    case hw_braille:
      hd_set_probe_feature(hd_data, pr_misc_serial);
      hd_set_probe_feature(hd_data, pr_serial);
      hd_set_probe_feature(hd_data, pr_braille_alva);
      hd_set_probe_feature(hd_data, pr_braille_fhp);
      hd_set_probe_feature(hd_data, pr_braille_ht);
      hd_set_probe_feature(hd_data, pr_braille_baum);
      break;

    case hw_sys:
      hd_set_probe_feature(hd_data, pr_bios);
      hd_set_probe_feature(hd_data, pr_prom);
      hd_set_probe_feature(hd_data, pr_sys);
      break;

    case hw_cpu:
      hd_set_probe_feature(hd_data, pr_cpu);
      break;

    case hw_bios:
      hd_set_probe_feature(hd_data, pr_bios);
      break;

    case hw_manual:
      hd_set_probe_feature(hd_data, pr_manual);
      break;

    case hw_usb_ctrl:
    case hw_pcmcia:
    case hw_pcmcia_ctrl:
    case hw_ieee1394_ctrl:
    case hw_hotplug_ctrl:
    case hw_ieee1394:
    case hw_hotplug:
      hd_set_probe_feature(hd_data, pr_misc);
      hd_set_probe_feature(hd_data, pr_pci);
      break;

    case hw_usb:
      hd_set_probe_feature(hd_data, pr_usb);
      hd_set_probe_feature(hd_data, pr_isdn);	// need pr_misc, too?
      hd_set_probe_feature(hd_data, pr_scsi);
      hd_set_probe_feature(hd_data, pr_partition);
      hd_data->flags.fast = 1;
      break;

    case hw_pci:
      hd_set_probe_feature(hd_data, pr_misc);
      hd_set_probe_feature(hd_data, pr_pci);
      hd_set_probe_feature(hd_data, pr_isdn);
#ifdef __PPC__
      hd_set_probe_feature(hd_data, pr_prom);
#endif
      break;

    case hw_isapnp:
      hd_set_probe_feature(hd_data, pr_isapnp);
      hd_set_probe_feature(hd_data, pr_isapnp_mod);
      hd_set_probe_feature(hd_data, pr_misc);
      hd_set_probe_feature(hd_data, pr_isdn);
      break;

    case hw_bridge:
      hd_set_probe_feature(hd_data, pr_misc);
      hd_set_probe_feature(hd_data, pr_pci);
      break;

    case hw_hub:
      hd_set_probe_feature(hd_data, pr_usb); 
      break;

    case hw_memory:
      hd_set_probe_feature(hd_data, pr_memory); 
      break;

    case hw_scsi:
      hd_set_probe_feature(hd_data, pr_usb);
      hd_set_probe_feature(hd_data, pr_scsi_cache);
      hd_set_probe_feature(hd_data, pr_ide);
      hd_set_probe_feature(hd_data, pr_partition);
      hd_set_probe_feature(hd_data, pr_cdrom);
      break;

    case hw_ide:
      hd_set_probe_feature(hd_data, pr_ide);
      hd_set_probe_feature(hd_data, pr_scsi);
      hd_set_probe_feature(hd_data, pr_cdrom);
      hd_set_probe_feature(hd_data, pr_partition);
      break;

    case hw_partition:
	      hd_set_probe_feature(hd_data, pr_partition);
	      break;

    case hw_all:
    case hw_unknown:
    case hw_zip:
      break;
  }
}


/*
 * Free all data associated with a hd_data_t struct. *Not* the struct itself.
 */
hd_data_t *hd_free_hd_data(hd_data_t *hd_data)
{
  hddb_pci_t *p;

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(hd_free_hd_data, hd_data), hd_data);
  }
#endif

  add_hd_entry2(&hd_data->old_hd, hd_data->hd); hd_data->hd = NULL;
  hd_data->log = free_mem(hd_data->log);
  free_old_hd_entries(hd_data);		/* hd_data->old_hd */
  /* hd_data->pci is always NULL */
  /* hd_data->isapnp->card is always NULL */
  hd_data->isapnp = free_mem(hd_data->isapnp);
  /* hd_data->cdrom is always NULL */
  hd_data->net = free_str_list(hd_data->net);
  hd_data->floppy = free_str_list(hd_data->floppy);
  hd_data->misc = free_misc(hd_data->misc);
  /* hd_data->serial is always NULL */
  /* hd_data->scsi is always NULL */
  /* hd_data->ser_mouse is always NULL */
  /* hd_data->ser_modem is always NULL */
  hd_data->cpu = free_str_list(hd_data->cpu);
  hd_data->klog = free_str_list(hd_data->klog);
  hd_data->proc_usb = free_str_list(hd_data->proc_usb);
  /* hd_data->usb is always NULL */

  if((p = hd_data->hddb_pci)) {
    for(; p->module; p++) free_mem(p->module);
  }
  if(hd_data->hddb2[0]) {
    free_mem(hd_data->hddb2[0]->list);
    free_mem(hd_data->hddb2[0]->ids); 
    free_mem(hd_data->hddb2[0]->strings);
    hd_data->hddb2[0] = free_mem(hd_data->hddb2[0]);
  }
  /* hddb2[1] is the static internal database; don't try to free it! */
  hd_data->hddb2[1] = NULL;

  hd_data->hddb_pci = free_mem(hd_data->hddb_pci);
  hd_data->kmods = free_str_list(hd_data->kmods);
  hd_data->bios_rom.data = free_mem(hd_data->bios_rom.data);
  hd_data->bios_ram.data = free_mem(hd_data->bios_ram.data);
  hd_data->bios_ebda.data = free_mem(hd_data->bios_ebda.data);
  hd_data->cmd_line = free_mem(hd_data->cmd_line);
  hd_data->xtra_hd = free_str_list(hd_data->xtra_hd);
  hd_data->devtree = free_devtree(hd_data);
  hd_data->manual = hd_free_manual(hd_data->manual);
  hd_data->disks = free_str_list(hd_data->disks);
  hd_data->partitions = free_str_list(hd_data->partitions);

  hd_data->smbios = free_smbios_list(hd_data->smbios);

  hd_data->last_idx = 0;

  return NULL;
}


/*
 * Free all data associated with a driver_info_t struct. Even the struct itself.
 */
driver_info_t *free_driver_info(driver_info_t *di)
{
  driver_info_t *next;

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(free_driver_info, di), di);
  }
#endif

  for(; di; di = next) {
    next = di->next;

    switch(di->any.type) {
      case di_any:
      case di_display:
        break;

      case di_module:
        free_str_list(di->module.names);
        free_str_list(di->module.mod_args);
        free_mem(di->module.conf);
        break;

      case di_mouse:
        free_mem(di->mouse.xf86);
        free_mem(di->mouse.gpm);
        break;

      case di_x11:
        free_mem(di->x11.server);
        free_mem(di->x11.xf86_ver);
        free_str_list(di->x11.extensions);
        free_str_list(di->x11.options);
        free_str_list(di->x11.raw);
        free_mem(di->x11.script);
        break;

      case di_isdn:
        free_mem(di->isdn.i4l_name);
        if(di->isdn.params) {
          isdn_parm_t *p = di->isdn.params, *next;
          for(; p; p = next) {
            next = p->next;
            free_mem(p->name);
            free_mem(p->alt_value);
            free_mem(p);
          }
        }
        break;

      case di_kbd:
        free_mem(di->kbd.XkbRules);
        free_mem(di->kbd.XkbModel);
        free_mem(di->kbd.XkbLayout);
        free_mem(di->kbd.keymap);
        break;
    }

    free_str_list(di->any.hddb0);
    free_str_list(di->any.hddb1);

    free_mem(di);
  }

  return NULL;
}


int exists_hd_entry(hd_data_t *hd_data, hd_t *old_hd, hd_t *hd_ex)
{
  hd_t *hd;

  if(!hd_ex) return 0;

  for(hd = hd_data->hd; hd; hd = hd->next) {
    if(hd == hd_ex) return 1;
  }
  for(hd = old_hd; hd; hd = hd->next) {
    if(hd == hd_ex) return 1;
  }

  return 0;
}


/*!
 * \note This may not free it.
 */
hd_t *hd_free_hd_list(hd_t *hd)
{
  hd_t *h;

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(hd_free_hd_list, hd), hd);
  }
#endif

  /* Note: hd->next should better be NULL! */
  if(hd && hd->tag.freeit) {
    free_hd_entry(hd);
    return free_mem(hd);
  }

  /* do nothing unless the list holds only copies of hd_t entries */
  for(h = hd; h; h = h->next) if(!h->ref) return NULL;

  for(; hd; hd = (h = hd)->next, free_mem(h));

  return NULL;
}

hd_detail_t *free_hd_detail(hd_detail_t *d)
{
  if(!d) return NULL;

  switch(d->type) {
    case hd_detail_pci: {
        pci_t *p = d->pci.data;

        free_mem(p->log);
        free_mem(p);
      }
      break;

    case hd_detail_usb:
      {
        usb_t *u = d->usb.data;

        free_str_list(u->b);
        free_str_list(u->c);
        free_str_list(u->ci);
        free_str_list(u->d);
        free_str_list(u->e);
        free_str_list(u->i);
        free_str_list(u->p);
        free_str_list(u->s);
        free_str_list(u->t);

        free_mem(u->manufact);
        free_mem(u->product);
        free_mem(u->serial);
        free_mem(u->driver);

        free_mem(u);
      }
      break;

    case hd_detail_isapnp:
      {
        isapnp_dev_t *i = d->isapnp.data;
        int j;

        if(!i->ref) {
          free_mem(i->card->serial);
          free_mem(i->card->card_regs);
          free_mem(i->card->ldev_regs);
          for(j = 0; j < i->card->res_len; j++) {
            free_mem(i->card->res[j].data);
          }
          if(i->card->res) free_mem(i->card->res);
        }
        free_mem(i->card);
        free_mem(i);
      }
      break;

    case hd_detail_cdrom:
      {
        cdrom_info_t *c = d->cdrom.data;

        free_mem(c->name);
        free_mem(c->iso9660.volume);
        free_mem(c->iso9660.publisher);
        free_mem(c->iso9660.preparer);
        free_mem(c->iso9660.application);
        free_mem(c->iso9660.creation_date);
        free_mem(c->el_torito.id_string);
        free_mem(c->el_torito.label);

        free_mem(c);
      }
      break;

    case hd_detail_floppy:
      free_mem(d->floppy.data);
      break;

    case hd_detail_bios:
      {
        bios_info_t *b = d->bios.data;

        free_mem(b->vbe.oem_name);
        free_mem(b->vbe.vendor_name);
        free_mem(b->vbe.product_name);
        free_mem(b->vbe.product_revision);
        free_mem(b->vbe.mode);
        free_mem(b->lcd.vendor);
        free_mem(b->lcd.name);
        free_mem(b->mouse.vendor);
        free_mem(b->mouse.type);

        free_mem(b);
      }
      break;

    case hd_detail_cpu:
      {
        cpu_info_t *c = d->cpu.data;

        free_mem(c->vend_name);
        free_mem(c->model_name);
        free_mem(c->platform);
        free_str_list(c->features);
        free_mem(c);
      }
      break;

    case hd_detail_prom:
      free_mem(d->prom.data);
      break;

    case hd_detail_monitor:
      {
        monitor_info_t *m = d->monitor.data;

        free_mem(m->vendor);
        free_mem(m->name);
        free_mem(m->serial);

        free_mem(m);
      }
      break;

    case hd_detail_sys:
      {
        sys_info_t *s = d->sys.data;

        free_mem(s->system_type);
        free_mem(s->generation);
        free_mem(s->vendor);
        free_mem(s->model);
        free_mem(s->serial);
        free_mem(s->lang);

        free_mem(s);
      }
      break;

    case hd_detail_scsi:
      free_scsi(d->scsi.data, 1);
      break;

    case hd_detail_devtree:
      /* is freed with hd_data->dev_tree */
      break;
  }

  free_mem(d);

  return NULL;
}


hd_t *free_hd_entry(hd_t *hd)
{
  free_mem(hd->bus.name);
  free_mem(hd->base_class.name);
  free_mem(hd->sub_class.name);
  free_mem(hd->prog_if.name);
  free_mem(hd->vendor.name);
  free_mem(hd->device.name);
  free_mem(hd->sub_vendor.name);
  free_mem(hd->sub_device.name);
  free_mem(hd->revision.name);
  free_mem(hd->serial);
  free_mem(hd->compat_vendor.name);
  free_mem(hd->compat_device.name);
  free_mem(hd->model);
  free_mem(hd->unix_dev_name);
  free_mem(hd->rom_id);
  free_mem(hd->unique_id);
  free_mem(hd->block0);
  free_mem(hd->driver);
  free_mem(hd->old_unique_id);
  free_mem(hd->unique_id1);
  free_mem(hd->usb_guid);
  free_mem(hd->parent_id);
  free_mem(hd->config_string);
  free_str_list(hd->extra_info);

  free_res_list(hd->res);

  free_hd_detail(hd->detail);

  free_driver_info(hd->driver_info);
  free_str_list(hd->requires);

  memset(hd, 0, sizeof *hd);

  return NULL;
}

misc_t *free_misc(misc_t *m)
{
  int i, j;

  if(!m) return NULL;

  for(i = 0; i < m->io_len; i++) {
    free_mem(m->io[i].dev);
  }
  free_mem(m->io);

  for(i = 0; i < m->dma_len; i++) {
    free_mem(m->dma[i].dev);
  }
  free_mem(m->dma);

  for(i = 0; i < m->irq_len; i++) {
    for(j = 0; j < m->irq[i].devs; j++) {
      free_mem(m->irq[i].dev[j]);
    }
    free_mem(m->irq[i].dev);
  }
  free_mem(m->irq);

  free_str_list(m->proc_io);
  free_str_list(m->proc_dma);
  free_str_list(m->proc_irq);

  free_mem(m);

  return NULL;
}

scsi_t *free_scsi(scsi_t *scsi, int free_all)
{
  scsi_t *next;

  for(; scsi; scsi = next) {
    next = scsi->next;

    free_mem(scsi->dev_name);
    free_mem(scsi->guessed_dev_name);
    free_mem(scsi->vendor);
    free_mem(scsi->model);
    free_mem(scsi->rev);
    free_mem(scsi->type_str);
    free_mem(scsi->serial);
    free_mem(scsi->proc_dir);
    free_mem(scsi->driver);
    free_mem(scsi->info);
    free_mem(scsi->usb_guid);
    free_str_list(scsi->host_info);

    if(!free_all) {
      next = scsi->next;
      memset(scsi, 0, sizeof scsi);
      scsi->next = next;
      break;
    }

    free_mem(scsi);
  }

  return NULL;
}


hd_manual_t *hd_free_manual(hd_manual_t *manual)
{
  hd_manual_t *next;

  if(!manual) return NULL;

  for(; manual; manual = next) {
    next = manual->next;

    free_mem(manual->unique_id);
    free_mem(manual->parent_id);
    free_mem(manual->model);

    free_mem(manual->config_string);

    free_str_list(manual->key);
    free_str_list(manual->value); 

    free_mem(manual); 
  }

  return NULL;
}


/*
 * Removes all hd_data->old_hd entries and frees their memory.
 */
void free_old_hd_entries(hd_data_t *hd_data)
{
  hd_t *hd, *next;

  for(hd = hd_data->old_hd; hd; hd = next) {
    next = hd->next;

    if(exists_hd_entry(hd_data, next, hd->ref) && hd->ref->ref_cnt) hd->ref->ref_cnt--;

    if(!hd->ref) free_hd_entry(hd);

    free_mem(hd);
  }

  hd_data->old_hd = NULL;
}


void *new_mem(size_t size)
{
  void *p;

  if(size == 0) return NULL;

  p = calloc(size, 1);

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log) fprintf(libhd_log, "%p\t%p\t0x%x\n", CALLED_FROM(new_mem, size), p, size);
  }
#endif

  if(p) return p;

  fprintf(stderr, "memory oops 1\n");
  exit(11);
  /*NOTREACHED*/
  return 0;
}

void *resize_mem(void *p, size_t n)
{
#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log && p) fprintf(libhd_log, "%p\t%p\n", CALLED_FROM(resize_mem, p), p);
  }
#endif

  p = realloc(p, n);

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log) fprintf(libhd_log, "%p\t%p\t0x%x\n", CALLED_FROM(resize_mem, p), p, n);
  }
#endif

  if(!p) {
    fprintf(stderr, "memory oops 7\n");
    exit(17);
  }

  return p;
}

void *add_mem(void *p, size_t elem_size, size_t n)
{
#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log && p) fprintf(libhd_log, "%p\t%p\n", CALLED_FROM(add_mem, p), p);
  }
#endif

  p = realloc(p, (n + 1) * elem_size);

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log) fprintf(libhd_log, "%p\t%p\t0x%x\n", CALLED_FROM(add_mem, p), p, (n + 1) * elem_size);
  }
#endif

  if(!p) {
    fprintf(stderr, "memory oops 7\n");
    exit(17);
  }

  memset(p + n * elem_size, 0, elem_size);

  return p;
}

char *new_str(const char *s)
{
  char *t;

  if(!s) return NULL;

  t = strdup(s);

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log) fprintf(libhd_log, "%p\t%p\t0x%x\n", CALLED_FROM(new_str, s), t, strlen(t) + 1);
  }
#endif

  if(t) return t;

  fprintf(stderr, "memory oops 2\n");
  /*NOTREACHED*/
  exit(12);

  return NULL;
}

void *free_mem(void *p)
{
#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log && p) fprintf(libhd_log, "%p\t%p\n", CALLED_FROM(free_mem, p), p);
  }
#endif

  if(p) free(p);

  return NULL;
}

void join_res_io(hd_res_t **res1, hd_res_t *res2)
{
  hd_res_t *res;

  /*
   * see if we must add an i/o range (tricky...)
   *
   * We look for identical i/o bases and add a range if one was missing. If
   * no matching pair was found, add the i/o resource.
   */
  for(; res2; res2 = res2->next) {
    if(res2->io.type == res_io) {
      for(res = *res1; res; res = res->next) {
        if(res->io.type == res_io) {
          if(res->io.base == res2->io.base) {
            /* identical bases: take maximum of both ranges */
            if(res2->io.range > res->io.range) {
              res->io.range = res2->io.range;
            }
            break;
          }
          else if(
            res->io.range &&
            res2->io.range &&
            res->io.base + res->io.range == res2->io.base)
          {
            /* res2 directly follows res1: extend res1 to cover res2 */
            res->io.range += res2->io.range;
            break;
          }
          else if(
            res2->io.base >= res->io.base &&
            res2->io.base < res->io.base + res->io.range
          ) {
            /* res2 is totally contained in res1: ignore it */
            break;
          }
        }
      }
      if(!res) {
        res = add_res_entry(res1, new_mem(sizeof *res));
        *res = *res2;	/* *copy* the struct */
        res->next = NULL;
      }
    }
  }
}

void join_res_irq(hd_res_t **res1, hd_res_t *res2)
{
  hd_res_t *res;

  /* see if we must add an dma channel */
  for(; res2; res2 = res2->next) {
    if(res2->irq.type == res_irq) {
      for(res = *res1; res; res = res->next) {
        if(res->irq.type == res_irq && res->irq.base == res2->irq.base) break;
      }
      if(!res) {
        res = add_res_entry(res1, new_mem(sizeof *res));
        *res = *res2;	/* *copy* the struct */
        res->next = NULL;
      }
    }
  }
}


void join_res_dma(hd_res_t **res1, hd_res_t *res2)
{
  hd_res_t *res;

  /* see if we must add an dma channel */
  for(; res2; res2 = res2->next) {
    if(res2->dma.type == res_dma) {
      for(res = *res1; res; res = res->next) {
        if(res->dma.type == res_dma && res->dma.base == res2->dma.base) break;
      }
      if(!res) {
        res = add_res_entry(res1, new_mem(sizeof *res));
        *res = *res2;	/* *copy* the struct */
        res->next = NULL;
      }
    }
  }
}


/*
 * Check whether both resource lists have common entries.
 */
int have_common_res(hd_res_t *res1, hd_res_t *res2)
{
  hd_res_t *res;

  for(; res1; res1 = res1->next) {
    for(res = res2; res; res = res->next) {
      if(res->any.type == res1->any.type) {
        switch(res->any.type) {
          case res_io:
            if(res->io.base == res1->io.base) return 1;
            break;

          case res_irq:
            if(res->irq.base == res1->irq.base) return 1;
            break;

          case res_dma:
            if(res->dma.base == res1->dma.base) return 1;
            break;

          default: /* gcc -Wall */
	    break;
        }
      }
    }
  }

  return 0;
}


/*
 * Free the memory allocated by a resource list.
 */
hd_res_t *free_res_list(hd_res_t *res)
{
  hd_res_t *next;

  for(; res; res = next) {
    next = res->next;

    if(res->any.type == res_init_strings) {
      free_mem(res->init_strings.init1);
      free_mem(res->init_strings.init2);
    }

    if(res->any.type == res_pppd_option) {
      free_mem(res->pppd_option.option);
    }

    free_mem(res);
  }

  return NULL;
}


/*
 * Note: new_res is directly inserted into the list, so you *must* make sure
 * that new_res points to a malloc'ed pice of memory.
 */
hd_res_t *add_res_entry(hd_res_t **res, hd_res_t *new_res)
{
  while(*res) res = &(*res)->next;

  return *res = new_res;
}


/*
 * Free the memory allocated by a smbios list.
 */
hd_smbios_t *free_smbios_list(hd_smbios_t *sm)
{
  hd_smbios_t *next;
  int i;

  for(; sm; sm = next) {
    next = sm->next;

    free_mem(sm->any.data);
    free_str_list(sm->any.strings);

    switch(sm->any.type) {
      case sm_biosinfo:
        free_mem(sm->biosinfo.vendor);
        free_mem(sm->biosinfo.version);
        free_mem(sm->biosinfo.date);
        break;

      case sm_sysinfo:
        free_mem(sm->sysinfo.manuf);
        free_mem(sm->sysinfo.product);
        free_mem(sm->sysinfo.version);
        free_mem(sm->sysinfo.serial);
        break;

      case sm_boardinfo:
        free_mem(sm->boardinfo.manuf);
        free_mem(sm->boardinfo.product);
        free_mem(sm->boardinfo.version);
        free_mem(sm->boardinfo.serial);
        break;

      case sm_chassis:
        free_mem(sm->chassis.manuf);
        break;

      case sm_processor:
        free_mem(sm->processor.socket);
        free_mem(sm->processor.manuf);
        free_mem(sm->processor.version);
        break;

      case sm_onboard:
        for(i = 0; i < sizeof sm->onboard.descr / sizeof *sm->onboard.descr; i++) {
          free_mem(sm->onboard.descr[i]);
        }
        break;

      case sm_lang:
        free_mem(sm->lang.current);
        break;

      case sm_memdevice:
        free_mem(sm->memdevice.location);
        free_mem(sm->memdevice.bank);
        break;

      default:
	break;
    }

    free_mem(sm);
  }

  return NULL;
}


/*
 * Note: new_smbios is directly inserted into the list, so you *must* make sure
 * that new_smbios points to a malloc'ed pice of memory.
 */
hd_smbios_t *add_smbios_entry(hd_smbios_t **sm, hd_smbios_t *new_sm)
{
  while(*sm) sm = &(*sm)->next;

  return *sm = new_sm;
}


hd_t *add_hd_entry(hd_data_t *hd_data, unsigned line, unsigned count)
{
  hd_t *hd;

  hd = add_hd_entry2(&hd_data->hd, new_mem(sizeof *hd));

  hd->idx = ++(hd_data->last_idx);
  hd->module = hd_data->module;
  hd->line = line;
  hd->count = count;

  return hd;
}


hd_t *add_hd_entry2(hd_t **hd, hd_t *new_hd)
{
  while(*hd) hd = &(*hd)->next;

  return *hd = new_hd;
}


void hd_scan(hd_data_t *hd_data)
{
  char *s = NULL;
  int i, j;
  hd_t *hd;
  uint64_t irqs;
  str_list_t *sl, *sl0;

#ifdef LIBHD_MEMCHECK
  if(!libhd_log) {
    char *s = getenv("LIBHD_MEMCHECK");

    if(s && *s) {
      libhd_log = fopen(s, "w");
      if(libhd_log) setlinebuf(libhd_log);
    }
  }
#endif

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(hd_scan, hd_data), hd_data);
  }
#endif

  /* log the debug & probe flags */
  if(hd_data->debug && !hd_data->flags.internal) {
    ADD2LOG("libhd version %s%s (%s)\n", HD_VERSION_STRING, getuid() ? "u" : "", HD_ARCH);
  }

  get_kernel_version(hd_data);

  /* needed only on 1st call */
  if(hd_data->last_idx == 0) {
    get_probe_env(hd_data);
  }

  fix_probe_features(hd_data);

  if(hd_data->debug && !hd_data->flags.internal) {
    for(i = sizeof hd_data->probe - 1; i >= 0; i--) {
      str_printf(&s, -1, "%02x", hd_data->probe[i]);
    }
    ADD2LOG("debug = 0x%x\nprobe = 0x%s (", hd_data->debug, s);
    s = free_mem(s);

    for(i = 1; i < pr_default; i++) {		/* 1 because of pr_memory */
      if((s = hd_probe_feature_by_value(i))) {
        ADD2LOG("%s%c%s", i == 1 ? "" : " ", hd_probe_feature(hd_data, i) ? '+' : '-', s);
      }
    }

    ADD2LOG(")\n");
  }

  /* init driver info database */
  hddb_init(hd_data);

#ifndef LIBHD_TINY
  /*
   * There might be old 'manual' entries left from an earlier scan. Remove
   * them, they will confuse us.
   */
  if(hd_probe_feature(hd_data, pr_manual)) {
    hd_data->module = mod_manual;
    remove_hd_entries(hd_data);
  }
#endif

  /*
   * for various reasons, do it befor scan_misc()
   */
  hd_scan_floppy(hd_data);

  /*
   * to be able to read the right parport io,
   * we have to do this before scan_misc()
   */
#if defined(__i386__) || defined (__x86_64__) || defined(__ia64__)
  hd_scan_bios(hd_data);
  //printf("scanned bios\n");
  //fflush(stdout);
#endif
  
  /* before hd_scan_misc(): we need some ppc info later */
  hd_scan_sys(hd_data);

  /* get basic system info */
  hd_scan_misc(hd_data);

  /* hd_scan_cpu() after hd_scan_misc(): klog needed */
  hd_scan_cpu(hd_data);
  hd_scan_memory(hd_data);

  hd_scan_pci(hd_data);

  /* do it _after_ hd_scan_pci() */
#if defined(__PPC__)
  hd_scan_prom(hd_data);
#endif

  /* after hd_scan_prom() and hd_scan_bios() */
  hd_scan_monitor(hd_data);

#ifndef LIBHD_TINY
#if defined(__i386__) || defined(__alpha__)
  hd_scan_isapnp(hd_data);
#endif
#endif

#ifndef LIBHD_TINY
#if defined(__i386__)
  hd_scan_isa(hd_data);
#endif
#endif

  hd_scan_serial(hd_data);

  /* merge basic system info & the easy stuff */
  hd_scan_misc2(hd_data);

#ifndef LIBHD_TINY
  if(!hd_data->flags.no_parport) {
    hd_scan_parallel(hd_data);	/* after hd_scan_misc*() */
  }
#endif
  /* do it rather early */
  hd_scan_partition(hd_data);

  hd_scan_ide(hd_data);
  hd_scan_scsi(hd_data);
  hd_scan_dac960(hd_data);
  hd_scan_smart(hd_data);
  hd_scan_i2o(hd_data);
  hd_scan_cciss(hd_data);
#if defined(__s390__) || defined(__s390x__)
  hd_scan_dasd(hd_data);
#endif
#if defined(__PPC__)   
  hd_scan_veth(hd_data);
#endif
  hd_scan_usb(hd_data);
#if defined(__PPC__)
  hd_scan_adb(hd_data);
#endif
  hd_scan_kbd(hd_data);
#ifndef LIBHD_TINY
#if !defined(__sparc__)
  hd_scan_braille(hd_data);
#endif
  hd_scan_modem(hd_data);	/* do it before hd_scan_mouse() */
  hd_scan_mouse(hd_data);
#endif
  hd_scan_sbus(hd_data);

  /* must be after hd_scan_monitor() */
  hd_scan_fb(hd_data);

  /* keep these at the end of the list */
  hd_scan_cdrom(hd_data);
  hd_scan_net(hd_data);
  hd_scan_disk(hd_data);
  hd_scan_ataraid(hd_data);

  for(hd = hd_data->hd; hd; hd = hd->next) hd_add_id(hd_data, hd);

#ifndef LIBHD_TINY
  hd_scan_manual(hd_data);
#endif

  /* add test entries */
  hd_scan_xtra(hd_data);

  /* some final fixup's */
#if WITH_ISDN
  hd_scan_isdn(hd_data);
#endif
  hd_scan_int(hd_data);

  hd_scan_cdrom2(hd_data);

  /* and again... */
  for(hd = hd_data->hd; hd; hd = hd->next) hd_add_id(hd_data, hd);

  /* assign a hw_class & build a useful model string */
  for(hd = hd_data->hd; hd; hd = hd->next) {
    assign_hw_class(hd_data, hd);
#ifndef LIBHD_TINY
    /* create model name _after_ hw_class */
    create_model_name(hd_data, hd);
#endif
  }

#ifndef LIBHD_TINY
  /* must be _after_ we have valid hw_class entries */
  hd_scan_manual2(hd_data);
#endif

  /* we are done... */
  for(hd = hd_data->hd; hd; hd = hd->next) hd->tag.fixed = 1;

  hd_data->module = mod_none;

  if(hd_data->debug && !hd_data->flags.internal) {
    sl0 = read_file(PROC_MODULES, 0, 0);
    ADD2LOG("----- /proc/modules -----\n");
    for(sl = sl0; sl; sl = sl->next) {
      ADD2LOG("  %s", sl->str);
    }
    ADD2LOG("----- /proc/modules end -----\n");
    free_str_list(sl0);
  }

  update_irq_usage(hd_data);

  if(hd_data->debug && !hd_data->flags.internal) {
    irqs = hd_data->used_irqs;

    ADD2LOG("  used irqs:");
    for(i = j = 0; i < 64; i++, irqs >>= 1) {
      if((irqs & 1)) {
        ADD2LOG("%c%d", j ? ',' : ' ', i);
        j = 1;
      }
    }
    ADD2LOG("\n");
  }
}


/*
 * Note: due to byte order problems decoding the id is really a mess...
 * And, we use upper case for hex numbers!
 */
char *isa_id2str(unsigned id)
{
  char *s = new_mem(8);
  unsigned u = ((id & 0xff) << 8) + ((id >> 8) & 0xff);
  unsigned v = ((id >> 8) & 0xff00) + ((id >> 24) & 0xff);

  s[0] = ((u >> 10) & 0x1f) + 'A' - 1;
  s[1] = ((u >>  5) & 0x1f) + 'A' - 1;
  s[2] = ( u        & 0x1f) + 'A' - 1;

  sprintf(s + 3, "%04X", v);

  return s;
}

char *eisa_vendor_str(unsigned v)
{
  static char s[4];

  s[0] = ((v >> 10) & 0x1f) + 'A' - 1;
  s[1] = ((v >>  5) & 0x1f) + 'A' - 1;
  s[2] = ( v        & 0x1f) + 'A' - 1;
  s[3] = 0;

  return s;
}


/*
 *  Must _not_ check that s is exactly 3 chars.
 */
unsigned name2eisa_id(char *s)
{
  int i;
  unsigned u = 0;

  for(i = 0; i < 3; i++) {
    u <<= 5;
    if(s[i] < 'A' - 1 || s[i] > 'A' - 1 + 0x1f) return 0;
    u += s[i] - 'A' + 1;
  }

  return MAKE_ID(TAG_EISA, u);
}


/*
 * Create a 'canonical' version, i.e. no spaces at start and end.
 *
 * Note: removes chars >= 0x80 as well (due to (char *))! This
 * is currently considered a feature.
 */
char *canon_str(char *s, int len)
{
  char *m2, *m1, *m0 = new_mem(len + 1);
  int i;

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log) fprintf(libhd_log, ">%p\n", CALLED_FROM(canon_str, s));
  }
#endif

  for(m1 = m0, i = 0; i < len; i++) {
    if(m1 == m0 && s[i] <= ' ') continue;
    *m1++ = s[i];
  }
  *m1 = 0;
  while(m1 > m0 && m1[-1] <= ' ') {
    *--m1 = 0;
  }

  m2 = new_str(m0);
  free_mem(m0);

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log) fprintf(libhd_log, "<%p\n", CALLED_FROM(canon_str, s));
  }
#endif

  return m2;
}


/*
 * Convert a n-digit hex number to its numerical value.
 */
int hex(char *s, int n)
{
  int i = 0, j;

  while(n--) {
    if(sscanf(s++, "%1x", &j) != 1) return -1;
    i = (i << 4) + j;
  }

  return i;
}


/* simple 32 bit fixed point numbers with n decimals */
int str2float(char *s, int n)
{
  int i = 0;
  int dot = 0;

  while(*s) {
    if(*s == '.') {
      if(dot++) return 0;
    }
    else if(*s >= '0' && *s <= '9') {
      if(dot) {
        if(!n) return i;
        n--;
      }
      i *= 10;
      i += *s - '0';
    }
    else {
      return 0;
    }

    s++;
  }

  while(n--) i *= 10;

  return i;
}


/* simple 32 bit fixed point numbers with n decimals */
char *float2str(int f, int n)
{
  int i = 1, j, m = n;
  static char buf[32];

  while(n--) i *= 10;

  j = f / i;
  i = f % i;

  while(i && !(i % 10)) i /= 10, m--;

  if(i) {
    sprintf(buf, "%d.%0*d", j, m, i);
  }
  else {
    sprintf(buf, "%d", j);
  }

  return buf;
}


/*
 * find hardware entry with given index
 */
hd_t *hd_get_device_by_idx(hd_data_t *hd_data, int idx)
{
  hd_t *hd;

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(hd_get_device_by_idx, hd_data), hd_data);
  }
#endif

  if(!idx) return NULL;		/* early out: idx is always != 0 */

  for(hd = hd_data->hd; hd; hd = hd->next) {
    if(hd->idx == idx) return hd;
  }

  return NULL;
}


/*
 * Give the actual name of the probing module.
 */
char *mod_name_by_idx(unsigned idx)
{
  unsigned u;

  for(u = 0; u < sizeof pr_modules / sizeof *pr_modules; u++)
    if(idx == pr_modules[u].val) return pr_modules[u].name;

  return "";
}


/*
 * Print to a string.
 * Note: *buf must point to a malloc'd memory area (or be NULL).
 *
 * Use an offset of -1 or -2 to append the new string.
 *
 * As this function is quite often used to extend our log messages, there
 * is a cache that holds the length of the last string we created. This way
 * we speed this up somewhat. Use an offset of -2 to use this feature.
 * Note: this only works as long as str_printf() is used *exclusively* to
 * extend the string.
 */
void str_printf(char **buf, int offset, char *format, ...)
{
  static char *last_buf = NULL;
  static int last_len = 0;
  int len, use_cache;
  char b[1024];
  va_list args;

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log) fprintf(libhd_log, ">%p\n", CALLED_FROM(str_printf, buf));
  }
#endif

  use_cache = offset == -2 ? 1 : 0;



  if(*buf) {
    if(offset == -1) {
      offset = strlen(*buf);
    }
    else if(offset == -2) {
      if(last_buf == *buf && last_len && !(*buf)[last_len])
        offset = last_len;
      else
        offset = strlen(*buf);
    }
  }
  else {
    offset = 0;
  }

  va_start(args, format);
  vsnprintf(b, sizeof b, format, args);
  va_end(args);
 
  //PhrozenSmoke addition - avoids seg-fault

  *buf=malloc((len = offset + strlen(b)) + 1);	
  //*buf = resize_mem(*buf, (len = offset + strlen(b)) + 1);
  strcpy(*buf + offset, b);

  if(use_cache) {
	/*if (last_buf != *buf)  {
  		printf("str_printf:   cached non-null\n");
  		fflush(stdout);
		last_buf=free_mem(last_buf);
							} */
    last_buf = *buf;
    last_len = len;
  }

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log) fprintf(libhd_log, "<%p\n", CALLED_FROM(str_printf, buf));
  }
#endif
}


void hexdump(char **buf, int with_ascii, unsigned data_len, unsigned char *data)
{
  unsigned i;

  for(i = 0; i < data_len; i++) {
    if(i)
      str_printf(buf, -2, " %02x", data[i]);
    else
      str_printf(buf, -2, "%02x", data[i]);
  }

  if(with_ascii) {
    str_printf(buf, -2, "  \"");
    for(i = 0; i < data_len; i++) {
      str_printf(buf, -2, "%c", data[i] < ' ' || data[i] >= 0x7f ? '.' : data[i]);
    }
    str_printf(buf, -2, "\"");
  }
}


/** \relates s_str_list_t
 * Search a string list for a string.
 */
str_list_t *search_str_list(str_list_t *sl, char *str)
{
  if(!str) return NULL;

  for(; sl; sl = sl->next) if(!strcmp(sl->str, str)) return sl;

  return NULL;
}


/** \relates s_str_list_t
 * Add a string to a string list.
 *
 * The new string (str) will be *copied*!
 */
str_list_t *add_str_list(str_list_t **sl, char *str)
{
#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log) fprintf(libhd_log, ">%p\n", CALLED_FROM(add_str_list, sl));
  }
#endif

  while(*sl) sl = &(*sl)->next;

  *sl = new_mem(sizeof **sl);
  (*sl)->str = new_str(str);

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log) fprintf(libhd_log, "<%p\n", CALLED_FROM(add_str_list, sl));
  }
#endif

  return *sl;
}


/** \relates s_str_list_t
 * Free the memory allocated by a string list.
 */
str_list_t *free_str_list(str_list_t *list)
{
  /* PhrozenSmoke modification: all commented out - was causing seg faults when re-scanning hardware */
  /*str_list_t *l;

  for(; list; list = (l = list)->next, free_mem(l)) {
    free_mem(list->str);
  }  */

  return NULL;
}


/*
 * Read a file; return a linked list of lines.
 *
 * start_line is zero-based; lines == 0 -> all lines
 */
str_list_t *read_file(char *file_name, unsigned start_line, unsigned lines)
{
  FILE *f;
  char buf[1024];
  int pipe = 0;
  str_list_t *sl_start = NULL, *sl_end = NULL, *sl;

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log) fprintf(libhd_log, ">%p\n", CALLED_FROM(read_file, file_name));
  }
#endif

  if(*file_name == '|') {
    pipe = 1;
    file_name++;
    if(!(f = popen(file_name, "r"))) {
#ifdef LIBHD_MEMCHECK
      {
        if(libhd_log) fprintf(libhd_log, "<%p\n", CALLED_FROM(read_file, file_name));
      }
#endif
      return NULL;
    }
  }
  else {
    if(!(f = fopen(file_name, "r"))) {
#ifdef LIBHD_MEMCHECK
      {
        if(libhd_log) fprintf(libhd_log, "<%p\n", CALLED_FROM(read_file, file_name));
      }
#endif
      return NULL;
    }
  }

  while(fgets(buf, sizeof buf, f)) {
    if(start_line) {
      start_line--;
      continue;
    }
    sl = new_mem(sizeof *sl);
    sl->str = new_str(buf);
    if(sl_start)
      sl_end->next = sl;
    else
      sl_start = sl;
    sl_end = sl;

    if(lines == 1) break;
    lines--;
  }

  if(pipe)
    pclose(f);
  else
    fclose(f);

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log) fprintf(libhd_log, "<%p\n", CALLED_FROM(read_file, file_name));
  }
#endif

  return sl_start;
}


/*
 * Read directory, return a list of entries with file type 'type'.
 */
str_list_t *read_dir(char *dir_name, int type)
{
  str_list_t *sl_start = NULL, *sl_end = NULL, *sl;
  DIR *dir;
  struct dirent *de;
  struct stat sbuf;
  char *s;
  int dir_type;

  if(dir_name && (dir = opendir(dir_name))) {
    while((de = readdir(dir))) {
      if(!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) continue;
      dir_type = 0;

      if(type) {
        s = NULL;
        str_printf(&s, 0, "%s/%s", dir_name, de->d_name);

        if(!lstat(s, &sbuf)) {
          if(S_ISDIR(sbuf.st_mode)) {
            dir_type = 'd';
          }
          else if(S_ISREG(sbuf.st_mode)) {
            dir_type = 'r';
          }
          else if(S_ISLNK(sbuf.st_mode)) {
            dir_type = 'l';
          }
        }

        s = free_mem(s);
      }

      if(dir_type == type) {
        sl = new_mem(sizeof *sl);
        sl->str = new_str(de->d_name);
        if(sl_start)
          sl_end->next = sl;
        else
          sl_start = sl;
        sl_end = sl;
      }
    }
    closedir(dir);
  }

  return sl_start;
}


char *hd_read_symlink(char *link_name)
{
  static char buf[256];
  int i;

  i = readlink(link_name, buf, sizeof buf);
  buf[sizeof buf - 1] = 0;
  if(i >= 0 && i < sizeof buf) buf[i] = 0;
  if(i < 0) *buf = 0;

  return buf;
}


/*
 * Log the hardware detection progress.
 */
void progress(hd_data_t *hd_data, unsigned pos, unsigned count, char *msg)
{
   return; 
  /*  DISABLED by Phrozensmoke for speed enhancements for IceWMCP */
  /*char buf1[32], buf2[32], buf3[128], *fn;

  if(!msg) msg = "";

  sprintf(buf1, "%u", hd_data->module);
  sprintf(buf2, ".%u", count);
  fn = mod_name_by_idx(hd_data->module);

  sprintf(buf3, "%s.%u%s", *fn ? fn : buf1, pos, count ? buf2 : "");

  if((hd_data->debug & HD_DEB_PROGRESS))
    ADD2LOG(">> %s: %s\n", buf3, msg);

  if(hd_data->progress) hd_data->progress(buf3, msg);   */
}



/*
 * Returns a probe feature suitable for hd_*probe_feature().
 * If name is not a valid probe feature, 0 is returned.
 *
 */
enum probe_feature hd_probe_feature_by_name(char *name)
{
  int u;

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(hd_probe_feature_by_name, name), name);
  }
#endif

  for(u = 0; u < sizeof pr_flags / sizeof *pr_flags; u++)
    if(!strcmp(name, pr_flags[u].name)) return pr_flags[u].val;

  return 0;
}


/*
 * Coverts a enum probe_feature to a string.
 * If it fails, NULL is returned.
 */
char *hd_probe_feature_by_value(enum probe_feature feature)
{
  int u;

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%u\n", __FUNCTION__, CALLED_FROM(hd_probe_feature_by_value, feature), feature);
  }
#endif

  for(u = 0; u < sizeof pr_flags / sizeof *pr_flags; u++)
    if(feature == pr_flags[u].val) return pr_flags[u].name;

  return NULL;
}


/*
 * Removes all hd_data->hd entries created by the current module from the
 * list. The old entries are added to hd_data->old_hd.
 */
void remove_hd_entries(hd_data_t *hd_data)
{
  hd_t *hd;

  for(hd = hd_data->hd; hd; hd = hd->next) {
    if(hd->module == hd_data->module) {
      hd->tag.remove = 1;
    }
  }

  remove_tagged_hd_entries(hd_data);
}


/*
 * Removes all hd_data->hd entries that have the remove tag set from the
 * list. The old entries are added to hd_data->old_hd.
 */
void remove_tagged_hd_entries(hd_data_t *hd_data)
{
  hd_t *hd, **prev, **h;

  for(hd = *(prev = &hd_data->hd); hd;) {
    if(hd->tag.remove) {
      /* find end of the old list... */
      h = &hd_data->old_hd;
      while(*h) h = &(*h)->next;
      *h = hd;		/* ...and append the entry */

      hd = *prev = hd->next;
      (*h)->next = NULL;
    }
    else {
      hd = *(prev = &hd->next);
    }
  }
}


int hd_module_is_active(hd_data_t *hd_data, char *mod)
{
  str_list_t *sl, *sl0 = read_kmods(hd_data);
  int active = 0;
#ifdef __PPC__
  char *s1, *s2;
#endif

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(hd_module_is_active, hd_data), hd_data);
  }
#endif

  for(sl = sl0; sl; sl = sl->next) {
    if(!strcmp(sl->str, mod)) break;
  }

  free_str_list(sl0);
  active = sl ? 1 : 0;

  if(active) return active;

#ifdef __PPC__
  /* temporary hack for ppc */
  if(!strcmp(mod, "gmac")) {
    s1 = "<6>eth";
    s2 = " GMAC ";
  }
  else if(!strcmp(mod, "mace")) {
    s1 = "<6>eth";
    s2 = " MACE ";
  }
  else if(!strcmp(mod, "bmac")) {
    s1 = "<6>eth";
    s2 = " BMAC";
  }
  else if(!strcmp(mod, "mac53c94")) {
    s1 = "<4>scsi";
    s2 = " 53C94";
  }
  else if(!strcmp(mod, "mesh")) {
    s1 = "<4>scsi";
    s2 = " MESH";
  }
  else if(!strcmp(mod, "swim3")) {
    s1 = "<6>fd";
    s2 = " SWIM3 ";
  }
  else {
    s1 = s2 = NULL;
  }

  if(s1) {
    for(sl = hd_data->klog; sl; sl = sl->next) {
      if(strstr(sl->str, s1) == sl->str && strstr(sl->str, s2)) {
        active = 1;
        break;
      }
    }
  }
#endif

  return active;
}


/*
 * cf. /usr/src/linux/drivers/block/ide-pci.c
 */
#ifndef PCI_DEVICE_ID_ARTOP_ATP860
#define PCI_DEVICE_ID_ARTOP_ATP860	0x0006
#endif

#ifndef PCI_DEVICE_ID_ARTOP_ATP860R
#define PCI_DEVICE_ID_ARTOP_ATP860R	0x0007
#endif

#ifndef PCI_DEVICE_ID_CMD_649
#define PCI_DEVICE_ID_CMD_649		0x0649
#endif

#ifndef PCI_DEVICE_ID_PROMISE_20267
#define PCI_DEVICE_ID_PROMISE_20267	0x4d30
#endif

#ifndef PCI_DEVICE_ID_INTEL_82443MX_1
#define PCI_DEVICE_ID_INTEL_82443MX_1	0x7199
#endif

#ifndef PCI_DEVICE_ID_INTEL_82372FB_1
#define PCI_DEVICE_ID_INTEL_82372FB_1	0x7601
#endif

#ifndef PCI_DEVICE_ID_INTEL_82820FW_5
#define PCI_DEVICE_ID_INTEL_82820FW_5	0x244b
#endif

#ifndef PCI_DEVICE_ID_AMD_VIPER_7409
#define PCI_DEVICE_ID_AMD_VIPER_7409	0x7409
#endif

#ifndef PCI_DEVICE_ID_CMD_648
#define PCI_DEVICE_ID_CMD_648		0x0648
#endif

#ifndef PCI_DEVICE_ID_TTI_HPT366
#define PCI_DEVICE_ID_TTI_HPT366	0x0004
#endif

#ifndef PCI_DEVICE_ID_PROMISE_20262
#define PCI_DEVICE_ID_PROMISE_20262	0x4d38
#endif

int hd_has_special_eide(hd_data_t *hd_data)
{
  int i;
  hd_t *hd;
  static unsigned ids[][2] = {
/* CONFIG_BLK_DEV_AEC62XX */
    { PCI_VENDOR_ID_ARTOP, PCI_DEVICE_ID_ARTOP_ATP850UF },
    { PCI_VENDOR_ID_ARTOP, PCI_DEVICE_ID_ARTOP_ATP860 },
    { PCI_VENDOR_ID_ARTOP, PCI_DEVICE_ID_ARTOP_ATP860R },
/* CONFIG_BLK_DEV_ALI15X3 */
    { PCI_VENDOR_ID_AL, PCI_DEVICE_ID_AL_M5229 },
/* CONFIG_BLK_DEV_AMD7409 */
    { PCI_VENDOR_ID_AMD, PCI_DEVICE_ID_AMD_VIPER_7409 },
/* CONFIG_BLK_DEV_CMD64X */
    { PCI_VENDOR_ID_CMD, PCI_DEVICE_ID_CMD_643 },
    { PCI_VENDOR_ID_CMD, PCI_DEVICE_ID_CMD_646 },
    { PCI_VENDOR_ID_CMD, PCI_DEVICE_ID_CMD_648 },
    { PCI_VENDOR_ID_CMD, PCI_DEVICE_ID_CMD_649 },
/* CONFIG_BLK_DEV_CY82C693 */
    { PCI_VENDOR_ID_CONTAQ, PCI_DEVICE_ID_CONTAQ_82C693 },
/* CONFIG_BLK_DEV_CS5530 */
    { PCI_VENDOR_ID_CYRIX, PCI_DEVICE_ID_CYRIX_5530_IDE },
/* CONFIG_BLK_DEV_HPT34X */
    { PCI_VENDOR_ID_TTI, PCI_DEVICE_ID_TTI_HPT343 },
/* CONFIG_BLK_DEV_HPT366 */
    { PCI_VENDOR_ID_TTI, PCI_DEVICE_ID_TTI_HPT366 },
/* CONFIG_BLK_DEV_NS87415 */
    { PCI_VENDOR_ID_NS, PCI_DEVICE_ID_NS_87410 },
    { PCI_VENDOR_ID_NS, PCI_DEVICE_ID_NS_87415 },
/* CONFIG_BLK_DEV_OPTI621 */
    { PCI_VENDOR_ID_OPTI, PCI_DEVICE_ID_OPTI_82C621 },
    { PCI_VENDOR_ID_OPTI, PCI_DEVICE_ID_OPTI_82C558 },
    { PCI_VENDOR_ID_OPTI, PCI_DEVICE_ID_OPTI_82C825 },
#if 0
/* CONFIG_BLK_DEV_PIIX */
    { PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371FB_0 },
    { PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371FB_1 },
    { PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371SB_1 },
    { PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB },
    { PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82801AB_1 },
    { PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82443MX_1 },
    { PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82801AA_1 },
    { PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82372FB_1 },
    { PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82451NX },
    { PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82820FW_5 },
#endif
/* CONFIG_BLK_DEV_PDC202XX */
    { PCI_VENDOR_ID_PROMISE, PCI_DEVICE_ID_PROMISE_20246 },
    { PCI_VENDOR_ID_PROMISE, PCI_DEVICE_ID_PROMISE_20262 },
    { PCI_VENDOR_ID_PROMISE, PCI_DEVICE_ID_PROMISE_20267 },
/* CONFIG_BLK_DEV_SIS5513 */
    { PCI_VENDOR_ID_SI, PCI_DEVICE_ID_SI_5513 },
/* CONFIG_BLK_DEV_TRM290 */
    { PCI_VENDOR_ID_TEKRAM, PCI_DEVICE_ID_TEKRAM_DC290 },
/* CONFIG_BLK_DEV_VIA82CXXX */
    { PCI_VENDOR_ID_VIA, PCI_DEVICE_ID_VIA_82C561 },
    { PCI_VENDOR_ID_VIA, PCI_DEVICE_ID_VIA_82C586_1 }
  };

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(hd_has_special_eide, hd_data), hd_data);
  }
#endif

  for(hd = hd_data->hd; hd; hd = hd->next) {
    if(hd->bus.id == bus_pci) {
      for(i = 0; i < sizeof ids / sizeof *ids; i++) {
        if(hd->vendor.id == ids[i][0] && hd->device.id == ids[i][1]) return 1;
      }
    }
  }

  return 0;
}


int hd_has_pcmcia(hd_data_t *hd_data)
{
  hd_t *hd;

  for(hd = hd_data->hd; hd; hd = hd->next) {
    if(is_pcmcia_ctrl(hd_data, hd)) return 1;
  }

  return 0;
}


int hd_apm_enabled(hd_data_t *hd_data)
{
  hd_t *hd;

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(hd_apm_enabled, hd_data), hd_data);
  }
#endif

  for(hd = hd_data->hd; hd; hd = hd->next) {
    if(
      hd->base_class.id == bc_internal &&
      hd->sub_class.id == sc_int_bios &&
      hd->detail &&
      hd->detail->type == hd_detail_bios &&
      hd->detail->bios.data
    ) {
      return hd->detail->bios.data->apm_enabled;
    }
  }

  return 0;
}


int hd_usb_support(hd_data_t *hd_data)
{
  hd_t *hd;
  hd_res_t *res;

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(hd_usb_support, hd_data), hd_data);
  }
#endif

  for(hd = hd_data->hd; hd; hd = hd->next) {
    if(hd->base_class.id == bc_serial && hd->sub_class.id == sc_ser_usb) {
      for(res = hd->res; res; res = res->next) {
        if(res->any.type == res_irq)
          return hd->prog_if.id == pif_usb_ohci ? 2 : 1;	/* 2: ohci, 1: uhci */
      }
    }
  }

  return 0;
}


int hd_smp_support(hd_data_t *hd_data)
{
  int is_smp = 0;
  unsigned u;
  hd_t *hd, *hd0;
#if defined(__i386__) || (!defined(LIBHD_TINY) && defined (__x86_64__))
  unsigned cpu_threads = 0;
#endif
#ifdef __i386__
  cpu_info_t *ct;
#endif

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(hd_smp_support, hd_data), hd_data);
  }
#endif

  u = hd_data->flags.internal;
  hd_data->flags.internal = 1;
  hd = hd_list(hd_data, hw_cpu, 0, NULL);
  if(!hd) hd = hd_list(hd_data, hw_cpu, 1, NULL);
  hd_data->flags.internal = u;

  for(is_smp = 0, hd0 = hd; hd0; hd0 = hd0->next) is_smp++;
  if(is_smp == 1) is_smp = 0;

#if defined(__i386__)
  if(
    hd &&
    hd->detail &&
    hd->detail->type == hd_detail_cpu &&
    (ct = hd->detail->cpu.data)
  ) {
    cpu_threads = ct->units;
  }
#endif

  hd = hd_free_hd_list(hd);

#if !defined(LIBHD_TINY) && (defined(__i386__) || defined (__x86_64__))
  if(is_smp < 2) {
    if(!hd_data->bios_ram.data) {
      hd_free_hd_list(hd_list(hd_data, hw_sys, 1, NULL));
    }
    is_smp = detect_smp(hd_data);
    // at least 2 processors
    if(is_smp < 2) is_smp = 0;
    if(!is_smp && cpu_threads > 1) is_smp = 2;
  }
#endif

#ifdef __PPC__
  if(is_smp < 2) {
    if(!hd_data->devtree) {
      hd_free_hd_list(hd_list(hd_data, hw_sys, 1, NULL));
    }
    is_smp = detect_smp(hd_data);
    if(is_smp < 0) is_smp = 0;
  }
#endif

#if defined(__s390__) || defined(__s390x__)
  if(!is_smp) is_smp = 1;
#endif

  return is_smp;
}


int hd_color(hd_data_t *hd_data)
{
#if 0
  hd_t *hd;
  prom_info_t *pt;

  for(hd = hd_data->hd; hd; hd = hd->next) {
    if(
      hd->base_class.id == bc_internal && hd->sub_class.id == sc_int_prom &&
      hd->detail && hd->detail->type == hd_detail_prom &&
      (pt = hd->detail->prom.data) &&
      pt->has_color
    ) {
      return pt->color;
    }
  }
#endif

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(hd_color, hd_data), hd_data);
  }
#endif

  if(hd_data->color_code) return hd_data->color_code & 0xffff;

  return -1;
}


int hd_mac_color(hd_data_t *hd_data)
{
#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(hd_mac_color, hd_data), hd_data);
  }
#endif

  return hd_color(hd_data);
}


unsigned hd_display_adapter(hd_data_t *hd_data)
{
  hd_t *hd;
  driver_info_t *di;
  unsigned disp, disp_sbus, disp_pci, disp_any, disp_di;
  unsigned disp_cnt, disp_any_cnt;

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(hd_display_adapter, hd_data), hd_data);
  }
#endif

  /* if we know exactly where our primary display is, return it */
  if(hd_get_device_by_idx(hd_data, hd_data->display)) return hd_data->display;

  disp = disp_sbus = disp_pci = disp_any = disp_di = 0;
  disp_cnt = disp_any_cnt = 0;

  for(hd = hd_data->hd; hd; hd = hd->next) {
    if(hd->base_class.id == bc_display) {
      disp_any_cnt++;
      if(!disp_any) disp_any = hd->idx;
      if(hd->sub_class.id == sc_dis_vga) {
        disp_cnt++;
        if(!disp) disp = hd->idx;
        if(hd->bus.id == bus_pci && !disp_pci) disp_pci = hd->idx;
        if(hd->bus.id == bus_sbus && !disp_sbus) disp_sbus = hd->idx;
      }
      if(!disp_di) {
        if(!(di = hd->driver_info)) {
          hddb_add_info(hd_data, hd);
          di = hd->driver_info;
        }
        if(di && di->any.type == di_x11 && di->x11.server) {
          disp_di = hd->idx;
        }
      }
    }
  }

  /* if there's only one display adapter, return it */
  if(disp_any_cnt == 1) return disp_any;

  /* if there's only one vga compatible adapter, return it */
  if(disp_cnt == 1) return disp;

  /* return 1st (vga compatible) sbus card */
  /* note: the sbus code enters display cards as 'vga compatible' */
  if(disp_sbus) return disp_sbus;

  /* return 1st display adapter that has some x11 info */
  if(disp_di) return disp_di;

  /* return 1st vga compatible pci card */
  if(disp_pci) return disp_pci;

  /* return 1st vga compatible card */
  if(disp) return disp;

  /* return 1st display adapter */
  if(disp_any) return disp_any;

  /* there were none... */
  return 0;
}


enum cpu_arch hd_cpu_arch(hd_data_t *hd_data)
{
  hd_t *hd;

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(hd_cpu_arch, hd_data), hd_data);
  }
#endif

  for(hd = hd_data->hd; hd; hd = hd->next) {
    if(
      hd->base_class.id == bc_internal &&
      hd->sub_class.id == sc_int_cpu &&
      hd->detail &&
      hd->detail->type == hd_detail_cpu &&
      hd->detail->cpu.data
    ) {
      return hd->detail->cpu.data->architecture;
    }
  }

#ifdef __i386__
  return arch_intel;
#else
#ifdef __alpha__
  return arch_alpha;
#else
#ifdef __PPC__
  return arch_ppc;
#else
#ifdef __sparc__
  return arch_sparc;
#else
#ifdef __s390x__
  return arch_s390x;
#else
#ifdef __s390__
  return arch_s390;
#else
#ifdef __ia64__
  return arch_ia64;
#else
#ifdef __x86_64__
  return arch_x86_64;
#else
  return arch_unknown;
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
}


enum boot_arch hd_boot_arch(hd_data_t *hd_data)
{
#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(hd_boot_arch, hd_data), hd_data);
  }
#endif

  return hd_data->boot;
}

/*
 * makes a (shallow) copy; does some magic fixes
 */
void hd_copy(hd_t *dst, hd_t *src)
{
  hd_t *tmp;
//  unsigned u;

  tmp = dst->next;
//  u = dst->idx;

  *dst = *src;
  src->ref_cnt++;
  dst->ref = src;

  dst->next = tmp;
//  dst->idx = u;

  /* needed to keep in sync with the real device tree */
  if(
    dst->detail &&
    dst->detail->type == hd_detail_devtree
  ) {
    dst->detail = NULL;		/* ??? was: free_mem(dst->detail); */
  }
}


hd_t *hd_list(hd_data_t *hd_data, hd_hw_item_t item, int rescan, hd_t *hd_old)
{
  hd_t *hd, *hd1, *hd_list = NULL;
  unsigned char probe_save[sizeof hd_data->probe];
  unsigned fast_save;

#ifdef LIBHD_MEMCHECK
#ifndef __PPC__
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\t%u\t%u\t%p\n", __FUNCTION__, CALLED_FROM(hd_list, hd_data), hd_data, item, rescan, hd_old);
  }
#endif
#endif

  if(rescan) {
    memcpy(probe_save, hd_data->probe, sizeof probe_save);
    fast_save = hd_data->flags.fast;
    hd_clear_probe_feature(hd_data, pr_all);
#ifdef __powerpc__
    hd_set_probe_feature(hd_data, pr_sys);
    hd_scan(hd_data);
#endif
    hd_set_probe_feature_hw(hd_data, item);
    hd_scan(hd_data);
    memcpy(hd_data->probe, probe_save, sizeof hd_data->probe);
    hd_data->flags.fast = fast_save;
  }

  for(hd = hd_data->hd; hd; hd = hd->next) {
    if(
      (
        hd->hw_class == item ||
        hd->hw_class2 == item ||
        hd->hw_class3 == item ||
        item == hw_manual
      )
#ifndef LIBHD_TINY
/* with LIBHD_TINY hd->status is not maintained (cf. manual.c) */
      && (
        hd->status.available == status_yes ||
        hd->status.available == status_unknown ||
        item == hw_manual ||
        hd_data->flags.list_all
      )
#endif
    ) {
      if(hd->is.softraiddisk) continue;		/* don't report them */

      /* don't report old entries again */
      for(hd1 = hd_old; hd1; hd1 = hd1->next) {
        if(!cmp_hd(hd1, hd)) break;
      }
      if(!hd1) {
        hd1 = add_hd_entry2(&hd_list, new_mem(sizeof *hd_list));
        hd_copy(hd1, hd);
      }
    }
  }

  if(item == hw_manual) {
    for(hd = hd_list; hd; hd = hd->next) {
      hd->status.available = hd->status.available_orig;
    }
  }

  return hd_list;
}


hd_t *hd_list_with_status(hd_data_t *hd_data, hd_hw_item_t item, hd_status_t status)
{
  hd_t *hd, *hd1, *hd_list = NULL;
  unsigned char probe_save[sizeof hd_data->probe];

#ifdef LIBHD_MEMCHECK
#ifndef __PPC__
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\t%u\n", __FUNCTION__, CALLED_FROM(hd_list_with_status, hd_data), hd_data, item);
  }
#endif
#endif

  memcpy(probe_save, hd_data->probe, sizeof probe_save);
  hd_clear_probe_feature(hd_data, pr_all);
  hd_set_probe_feature(hd_data, pr_manual);
  hd_scan(hd_data);
  memcpy(hd_data->probe, probe_save, sizeof hd_data->probe);

  for(hd = hd_data->hd; hd; hd = hd->next) {
    if(hd->hw_class == item || hd->hw_class2 == item || hd->hw_class3 == item) {
      if(
        (status.configured == 0 || status.configured == hd->status.configured) &&
        (status.available == 0 || status.available == hd->status.available) &&
        (status.needed == 0 || status.needed == hd->status.needed) &&
        (status.reconfig == 0 || status.reconfig == hd->status.reconfig)
      ) {
        hd1 = add_hd_entry2(&hd_list, new_mem(sizeof *hd_list));
        hd_copy(hd1, hd);
      }
    }
  }

  return hd_list;
}


hd_t *hd_base_class_list(hd_data_t *hd_data, unsigned base_class)
{
  hd_t *hd, *hd1, *hd_list = NULL;
//  hd_t *bridge_hd;

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(hd_base_class_list, hd_data), hd_data);
  }
#endif

  for(hd = hd_data->hd; hd; hd = hd->next) {

#if 0
    /* ###### fix later: card bus magic */
    if((bridge_hd = hd_get_device_by_idx(hd_data, hd->attached_to))) {
      if(
        bridge_hd->base_class.id == bc_bridge &&
        bridge_hd->sub_class.id == sc_bridge_cardbus
      ) continue;
    }
#endif

    /* add multimedia/sc_multi_video to display */
    if(
      hd->base_class.id == base_class ||
      (
        base_class == bc_display &&
        hd->base_class.id == bc_multimedia &&
        hd->sub_class.id == sc_multi_video
      )
    ) {
      hd1 = add_hd_entry2(&hd_list, new_mem(sizeof *hd_list));
      hd_copy(hd1, hd);
    }
  }

  return hd_list;
}

hd_t *hd_sub_class_list(hd_data_t *hd_data, unsigned base_class, unsigned sub_class)
{
  hd_t *hd, *hd1, *hd_list = NULL;

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(hd_sub_class_list, hd_data), hd_data);
  }
#endif

  for(hd = hd_data->hd; hd; hd = hd->next) {
    if(hd->base_class.id == base_class && hd->sub_class.id == sub_class) {
      hd1 = add_hd_entry2(&hd_list, new_mem(sizeof *hd_list));
      hd_copy(hd1, hd);
    }
  }

  return hd_list;
}

hd_t *hd_bus_list(hd_data_t *hd_data, unsigned bus)
{
  hd_t *hd, *hd1, *hd_list = NULL;

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(hd_bus_list, hd_data), hd_data);
  }
#endif

  for(hd = hd_data->hd; hd; hd = hd->next) {
    if(hd->bus.id == bus) {
      hd1 = add_hd_entry2(&hd_list, new_mem(sizeof *hd_list));
      hd_copy(hd1, hd);
    }
  }

  return hd_list;
}

/*
 * Check if the execution of (*func)() takes longer than timeout seconds.
 * This is useful to work around long kernel-timeouts as in the floppy
 * detection and ps/2 mouse detection.
 */
int hd_timeout(void(*func)(void *), void *arg, int timeout)
{
  int child1, child2;
  int status = 0;

  child1 = fork();
  if(child1 == -1) return -1;

  if(child1) {
    if(waitpid(child1, &status, 0) == -1) return -1;
//    fprintf(stderr, ">child1 status: 0x%x\n", status);

    if(WIFEXITED(status)) {
      status = WEXITSTATUS(status);
//      fprintf(stderr, ">normal child1 status: 0x%x\n", status);
      /* != 0 if we timed out */
    }
    else {
      status = 0;
    }
  }
  else {
    /* fork again */

#ifdef LIBHD_MEMCHECK
    /* stop logging in child process */
    if(libhd_log) fclose(libhd_log);
    libhd_log = NULL;
#endif

    child2 = fork();
    if(child2 == -1) return -1;

    if(child2) {
//      fprintf(stderr, ">signal\n");
      signal(SIGALRM, timeout_alarm_handler);
      alarm(timeout);
      if(waitpid(child2, &status, 0) == -1) return -1;
//      fprintf(stderr, ">child2 status: 0x%x\n", status);
      _exit(0);
    }
    else {
      (*func)(arg);
      _exit(0);
    }
  }

  return status ? 1 : 0;
}

void timeout_alarm_handler(int signal)
{
  _exit(63);
}


str_list_t *read_kmods(hd_data_t *hd_data)
{
  str_list_t *sl, *sl0, *sl1 = NULL;
  char *s;

  hd_data->kmods = free_str_list(hd_data->kmods);

  if(!(sl0 = read_file(PROC_MODULES, 0, 0))) return NULL;

  hd_data->kmods = sl0;

  for(sl = sl0; sl; sl = sl->next) {
    s = sl->str;
    add_str_list(&sl1, strsep(&s, " \t"));
  }

  return sl1;
}


char *get_cmdline(hd_data_t *hd_data, char *key)
{
  str_list_t *sl0, *sl1;
  char *s, *t, *t0;
  int i, l = strlen(key);

  if(!hd_data->cmd_line) {
    sl0 = read_file(PROC_CMDLINE, 0, 1);
    sl1 = read_file(LIB_CMDLINE, 0, 1);

    if(sl0) {
      i = strlen(sl0->str);
      if(i && sl0->str[i - 1] == '\n') sl0->str[i - 1] = 0;
      hd_data->cmd_line = new_str(sl0->str);
      if(hd_data->debug) {
        ADD2LOG("----- " PROC_CMDLINE " -----\n");
        ADD2LOG("  %s\n", sl0->str);
        ADD2LOG("----- " PROC_CMDLINE " end -----\n");
      }
    }
    if(sl1) {
      i = strlen(sl1->str);
      if(i && sl1->str[i - 1] == '\n') sl1->str[i - 1] = 0;
      str_printf(&hd_data->cmd_line, -1, " %s", sl1->str);
      if(hd_data->debug) {
        ADD2LOG("----- " LIB_CMDLINE " -----\n");
        ADD2LOG("  %s\n", sl1->str);
        ADD2LOG("----- " LIB_CMDLINE " end -----\n");
      }
    }

    free_str_list(sl0);
    free_str_list(sl1);
  }

  if(!hd_data->cmd_line) return NULL;

  t = t0 = new_str(hd_data->cmd_line);
  while((s = strsep(&t, " "))) {
    if(!*s) continue;
    if(!strncmp(s, key, l) && s[l] == '=') {
      s += l + 1;
      break;
    }
  }

  s = new_str(s);

  free_mem(t0);

  return s;
}


/*
 * Return field 'field' (starting with 0) from the 'SuSE='
 * kernel cmd line parameter.
 */
char *get_cmd_param(hd_data_t *hd_data, int field)
{
  char *s, *t, *t0;

  s = t0 = get_cmdline(hd_data, "SuSE");

  if(!t0) return NULL;

  t = NULL;

  if(s) {
    for(; field; field--) {
      if(!(s = strchr(s, ','))) break;
      s++;
    }

    if(s && (t = strchr(s, ','))) *t = 0;
  }

  t = new_str(s);

  free_mem(t0);

  return t;
}


unsigned get_disk_crc(unsigned char *data, unsigned len)
{
  unsigned i, crc;

  crc = -1;
  for(i = 0; i < len; i++) {
    crc += data[i];
    crc *= 57;
  }

  return crc;
}

disk_t *add_disk_entry(disk_t **dl, disk_t *new_dl)
{
  while(*dl) dl = &(*dl)->next;
  return *dl = new_dl;
}

disk_t *free_disk_list(disk_t *dl)
{
  disk_t *l;

  for(; dl; dl = (l = dl)->next, free_mem(l));

  return NULL;
}

int dev_name_duplicate(disk_t *dl, char *dev_name)
{
  for(; dl; dl = dl->next) {
    if(!strcmp(dl->dev_name, dev_name)) return 1;
  }

  return 0;
}

unsigned hd_boot_disk(hd_data_t *hd_data, int *matches)
{
  hd_t *hd;
  unsigned crc, hd_idx = 0;
  char *s;
  int i, j;
  disk_t *dl, *dl0 = NULL, *dl1 = NULL;

#ifdef LIBHD_MEMCHECK
  {
    if(libhd_log)
      fprintf(libhd_log, "; %s\t%p\t%p\n", __FUNCTION__, CALLED_FROM(hd_boot_disk, hd_data), hd_data);
  }
#endif

  if(matches) *matches = 0;

  if(!(s = get_cmd_param(hd_data, 2))) return 0;

  i = strlen(s);

  if(i >= 8) {
    crc = hex(s, 8);
  }
  else {
    free_mem(s);
    return 0;
  }

  s = free_mem(s);

  if((hd_data->debug & HD_DEB_BOOT)) {
    ADD2LOG("    boot dev crc 0x%x\n", crc);
  }

  for(hd = hd_data->hd; hd; hd = hd->next) {
    if(
      hd->base_class.id == bc_storage_device &&
      hd->sub_class.id == sc_sdev_disk &&
      hd->block0
    ) {
      if(dev_name_duplicate(dl0, hd->unix_dev_name)) continue;
      dl = add_disk_entry(&dl0, new_mem(sizeof *dl0));
      dl->dev_name = hd->unix_dev_name;
      dl->hd_idx = hd->idx;
      dl->crc = get_disk_crc(dl->data = hd->block0, 512);
    }
  }

  if(!dl0) return 0;

  if((hd_data->debug & HD_DEB_BOOT)) {
    for(dl = dl0; dl; dl = dl->next) {
      ADD2LOG("    crc %s 0x%08x\n", dl->dev_name, dl->crc);
    }
  }

  for(i = 0, dl = dl0; dl; dl = dl->next) {
    if(crc == dl->crc) {
      dl->crc_match = 1;
      dl1 = dl;
      if(!i++) hd_idx = dl->hd_idx;
    }
  }

  if(i == 1 && dl1 && (hd_data->debug & HD_DEB_BOOT)) {
    ADD2LOG("----- MBR -----\n");
    for(j = 0; j < 512; j += 0x10) {
      ADD2LOG("  %03x  ", j);
      hexdump(&hd_data->log, 1, 0x10, dl1->data + j);
      ADD2LOG("\n");
    }
    ADD2LOG("----- MBR end -----\n");
  }

  free_disk_list(dl0);

  if(matches) *matches = i;

  hd_data->debug &= ~HD_DEB_BOOT;

  return hd_idx;
}

void update_irq_usage(hd_data_t *hd_data)
{
  hd_t *hd;
  misc_irq_t *mi;
  unsigned u, v;
  uint64_t irqs = 0;
  hd_res_t *res;

  if(hd_data->misc) {
    mi = hd_data->misc->irq;
    for(u = 0; u < hd_data->misc->irq_len; u++) {
      v = mi[u].irq;
      irqs |= 1ull << v;
    }
  }

  for(hd = hd_data->hd; hd; hd = hd->next) {
    for(res = hd->res; res; res = res->next) {
      if(res->any.type == res_irq) {
        irqs |= 1ull << res->irq.base;
      }
    }
  }

  hd_data->used_irqs = irqs;
}

int run_cmd(hd_data_t *hd_data, char *cmd)
{
  char *xcmd = NULL;
  str_list_t *sl, *sl0;

  ADD2LOG("----- exec: \"%s\" -----\n", cmd);

  if(*cmd == '/') {
    str_printf(&xcmd, 0, "|%s 2>&1", cmd);
    sl0 = read_file(xcmd, 0, 0);
    for(sl = sl0; sl; sl = sl->next) ADD2LOG("  %s", sl->str);
    sl0 = free_str_list(sl0);
  }

  ADD2LOG("----- return code: ? -----\n");

  free_mem(xcmd);

  return 0;
}

int probe_module(hd_data_t *hd_data, char *module)
{
  char *cmd = NULL;
  int i;

  if(hd_module_is_active(hd_data, module)) return 0;

  str_printf(&cmd, 0, "/sbin/modprobe %s", module);

  i = run_cmd(hd_data, cmd);

  free_mem(cmd);

  return i;
}

int load_module_with_params(hd_data_t *hd_data, char *module, char *params)
{
  char *cmd = NULL;
  int i;

  if(hd_module_is_active(hd_data, module)) return 0;

  str_printf(&cmd, 0, "/sbin/insmod %s %s", module, params ? params : "");

  i = run_cmd(hd_data, cmd);

  free_mem(cmd);

  return i;
}

int load_module(hd_data_t *hd_data, char *module)
{
  return load_module_with_params(hd_data, module, NULL);
}

int unload_module(hd_data_t *hd_data, char *module)
{
  char *cmd = NULL;
  int i;

  if(!hd_module_is_active(hd_data, module)) return 0;

  str_printf(&cmd, 0, "/sbin/rmmod %s", module);

  i = run_cmd(hd_data, cmd);

  free_mem(cmd);
  
  return i;
}

/*
 * Compare two hd entries and return 0 if they are identical.
 */
int cmp_hd(hd_t *hd1, hd_t *hd2)
{
  if(!hd1 || !hd2) return 1;

  if(
    hd1->bus.id != hd2->bus.id ||
    hd1->slot != hd2->slot ||
    hd1->func != hd2->func ||
    hd1->base_class.id != hd2->base_class.id ||
    hd1->sub_class.id != hd2->sub_class.id ||
    hd1->prog_if.id != hd2->prog_if.id ||
    hd1->device.id != hd2->device.id ||
    hd1->vendor.id != hd2->vendor.id ||
    hd1->sub_vendor.id != hd2->sub_vendor.id ||
    hd1->revision.id != hd2->revision.id ||
    hd1->compat_device.id != hd2->compat_device.id ||
    hd1->compat_vendor.id != hd2->compat_vendor.id ||

    hd1->module != hd2->module ||
    hd1->line != hd2->line
  ) {
    return 1;
  }

  if(hd1->unix_dev_name || hd2->unix_dev_name) {
    if(hd1->unix_dev_name && hd2->unix_dev_name) {
      if(strcmp(hd1->unix_dev_name, hd2->unix_dev_name)) return 1;
    }
    else {
      return 1;
    }
  }

  return 0;
}


void get_probe_env(hd_data_t *hd_data)
{
  char *s, *t, *env;
  int j, k;
  char buf[10];

  env = getenv("hwprobe");
  if(!env) env = get_cmdline(hd_data, "hwprobe");
  s = env = new_str(env);
  if(!env) return;

  hd_data->xtra_hd = free_str_list(hd_data->xtra_hd);

  while((t = strsep(&s, ","))) {
    if(*t == '+') {
      k = 1; t++;
    }
    else if(*t == '-') {
      k = 0; t++;
    }
    else {
      k = 2;
//      ADD2LOG("hwprobe: +/- missing before \"%s\"\n", t);
//      return;
    }

    if((j = hd_probe_feature_by_name(t))) {
      set_probe_feature(hd_data, j, k ? 1 : 0);
    }
    else if(sscanf(t, "%8[^:]:%8[^:]:%8[^:]", buf, buf, buf) == 3) {
      add_str_list(&hd_data->xtra_hd, t - (k == 2 ? 0 : 1));
    }
    else {
      ADD2LOG("hwprobe: what is \"%s\"?\n", t);
      return;
    }
  }

  free_mem(env);
}

void hd_scan_xtra(hd_data_t *hd_data)
{
  str_list_t *sl;
  hd_t *hd, *hd_tmp;
  unsigned u0, u1, u2, tag;
  int i, err;
  char buf0[10], buf1[10], buf2[10], buf3[64], *s, k;

  hd_data->module = mod_xtra;

  remove_hd_entries(hd_data);

  for(sl = hd_data->xtra_hd; sl; sl = sl->next) {
    s = sl->str;
    err = 0;
    switch(*s) {
      case '+': k = 1; s++; break;
      case '-': k = 0; s++; break;
      default: k = 2;
    }
    if(
      (i = sscanf(s, "%8[^:]:%8[^:]:%8[^:]:%60s", buf0, buf1, buf2, buf3)) >= 3
    ) {
      if(i < 4) *buf3 = 0;

      u0 = strtoul(buf0, &s, 16);
      if(*s) err |= 1;
      if(strlen(buf1) == 3) {
        u1 = name2eisa_id(buf1);
      }
      else {
        tag = TAG_PCI;
        s = buf1;
        switch(*s) {
          case 'p': tag = TAG_PCI; s++; break;
          case 'r': tag = 0; s++; break;
          case 's': tag = TAG_SPECIAL; s++; break;
          case 'u': tag = TAG_USB; s++; break;
        }
        u1 = strtoul(s, &s, 16);
        if(*s) err |= 2;
        u1 = MAKE_ID(tag, u1);
      }
      u2 = strtoul(buf2, &s, 16);
      if(*s) err |= 4;
      u2 = MAKE_ID(ID_TAG(u1), ID_VALUE(u2));
      if((err & 1) && !strcmp(buf0, "*")) {
        u0 = -1;
        err &= ~1;
      }
      if((err & 2) && !strcmp(buf1, "*")) {
        u1 = 0;
        err &= ~2;
      }
      if((err & 4) && !strcmp(buf2, "*")) {
        u2 = 0;
        err &= ~4;
      }
      if(!err) {
        if(k) {
          if(k == 2) {
            /* insert at top */
            hd_tmp = hd_data->hd;
            hd_data->hd = NULL;
            hd = add_hd_entry(hd_data, __LINE__, 0);
            hd->next = hd_tmp;
            hd_tmp = NULL;
          }
          else {
            hd = add_hd_entry(hd_data, __LINE__, 0);
          }
          hd->base_class.id = u0 >> 8;
          hd->sub_class.id = u0 & 0xff;
          hd->vendor.id = u1;
          hd->device.id = u2;
          if(ID_TAG(hd->vendor.id) == TAG_PCI) hd->bus.id = bus_pci;
          if(ID_TAG(hd->vendor.id) == TAG_USB) hd->bus.id = bus_usb;
          if(*buf3) hd->unix_dev_name = new_str(buf3);
          hd->status.available = status_yes;
          hd->status.configured = status_new;
          hd->status.needed = status_no;
        }
        else {
          for(hd = hd_data->hd; hd; hd = hd->next) {
            if(
                (u0 == -1 || (
                  hd->base_class.id == (u0 >> 8) &&
                  hd->sub_class.id == (u0 & 0xff)
                )) &&
                (u1 == 0 || hd->vendor.id == u1) &&
                (u2 == 0 || hd->device.id == u2) &&
                (*buf3 == 0 || (
                  hd->unix_dev_name &&
                  !strcmp(hd->unix_dev_name, buf3)
                ))
            ) {
              hd->tag.remove = 1;
            }
          }
          remove_tagged_hd_entries(hd_data);
        }
      }
    }
  }
}

unsigned has_something_attached(hd_data_t *hd_data, hd_t *hd)
{
  hd_t *hd1;

  for(hd1 = hd_data->hd; hd1; hd1 = hd1->next) {
    if(hd1->attached_to == hd->idx) return hd1->idx;
  }

  return 0;
}


/* ##### FIX: replace with a real crc later ##### */
void crc64(uint64_t *id, void *p, int len)
{
  unsigned char uc;

  for(; len; len--, p++) {
    uc = *(unsigned char *) p;
    *id += uc + ((uc + 57) << 27);
    *id *= 73;
    *id *= 65521;
  }
}

char *numid2str(uint64_t id, int len)
{
  static char buf[32];

#ifdef NUMERIC_UNIQUE_ID
  /* numeric */

  if(len < (sizeof id << 3)) id &= ~(-1LL << len);
  sprintf(buf, "%0*"PRIx64, len >> 2, id);

#else
  /* base64 like */

  int i;
  unsigned char u;

  memset(buf, 0, sizeof buf);
  for(i = 0; len > 0 && i < sizeof buf - 1; i++, len -= 6, id >>= 6) {
    u = id & 0x3f;
    if(u < 10) {
      u += '0';			/* 0..9 */
    }
    else if(u < 10 + 26) {
      u += 'A' - 10;		/* A..Z */
    }
    else if(u < 10 + 26 + 26) {
      u += 'a' - 10 - 26;	/* a..z */
    }
    else if(u == 63) {
      u = '+';
    }
    else {
      u = '_';
    }
    buf[i] = u;
  }

#endif

  return buf;
}

/*
 * calculate unique ids
 */
#define INT_CRC(a, b)	crc64(&a, &hd->b, sizeof hd->b);
#define STR_CRC(a, b)	if(hd->b) crc64(&a, hd->b, strlen(hd->b) + 1);


// old method
void hd_add_old_id(hd_t *hd)
{
  uint64_t id0 = 0, id1 = 0;

  if(hd->unique_id) return;

  INT_CRC(id0, bus.id);
  INT_CRC(id0, slot);
  INT_CRC(id0, func);
  INT_CRC(id0, base_class.id);
  INT_CRC(id0, sub_class.id);
  INT_CRC(id0, prog_if.id);
  STR_CRC(id0, unix_dev_name);
  STR_CRC(id0, rom_id);

  INT_CRC(id1, base_class.id);
  INT_CRC(id1, sub_class.id);
  INT_CRC(id1, prog_if.id);
  INT_CRC(id1, device.id);
  INT_CRC(id1, vendor.id);
  INT_CRC(id1, sub_device.id);
  INT_CRC(id1, sub_vendor.id);
  INT_CRC(id1, revision.id);
  INT_CRC(id1, compat_device.id);
  INT_CRC(id1, compat_vendor.id);
  STR_CRC(id1, device.name);
  STR_CRC(id1, vendor.name);
  STR_CRC(id1, sub_device.name);
  STR_CRC(id1, sub_vendor.name);
  STR_CRC(id1, revision.name);
  STR_CRC(id1, serial);

  id0 += (id0 >> 32);
  str_printf(&hd->unique_id, 0, "%s", numid2str(id0, 24));
  str_printf(&hd->unique_id, -1, ".%s", numid2str(id1, 64));
}

void hd_add_id(hd_data_t *hd_data, hd_t *hd)
{
  uint64_t id0 = 0, id1 = 0;
  hd_t *hd1;
  int cnt;

  if(hd->unique_id) return;

  hd_add_old_id(hd);
  hd->old_unique_id = hd->unique_id;
  hd->unique_id = NULL;

  INT_CRC(id1, base_class.id);
  INT_CRC(id1, sub_class.id);
  INT_CRC(id1, prog_if.id);
  INT_CRC(id1, device.id);
  INT_CRC(id1, vendor.id);
  INT_CRC(id1, sub_device.id);
  INT_CRC(id1, sub_vendor.id);
  INT_CRC(id1, revision.id);
  INT_CRC(id1, compat_device.id);
  INT_CRC(id1, compat_vendor.id);
  // make sure we get the same id even if, say, the pci name list changes
  if(!hd->device.id) STR_CRC(id1, device.name);
  if(!hd->vendor.id) STR_CRC(id1, vendor.name);
  if(!hd->sub_device.name) STR_CRC(id1, sub_device.name);
  if(!hd->sub_vendor.name) STR_CRC(id1, sub_vendor.name);
  if(!hd->revision.name) STR_CRC(id1, revision.name);
  STR_CRC(id1, serial);

  hd->unique_id1 = new_str(numid2str(id1, 64));

  INT_CRC(id0, bus.id);
  // usb likes to re-attach devices at different places
  if(hd->unix_dev_name) {
    STR_CRC(id0, unix_dev_name);
  }
  else if(
    hd->bus.id == bus_usb
  ) {
    for(cnt = 0, hd1 = hd_data->hd; hd1 && hd1->idx != hd->idx; hd1 = hd1->next) {
      if(
        hd1->unique_id1 &&
        !hd1->tag.remove &&
        !strcmp(hd1->unique_id1, hd->unique_id1)
      ) cnt++;
    }
    crc64(&id0, &cnt, sizeof cnt);
  }
  else {
    INT_CRC(id0, slot);
    INT_CRC(id0, func);
  }
  STR_CRC(id0, rom_id);

  id0 += (id0 >> 32);

  str_printf(&hd->unique_id, 0, "%s.%s", numid2str(id0, 24), hd->unique_id1);
}
#undef INT_CRC
#undef STR_CRC


devtree_t *free_devtree(hd_data_t *hd_data)
{
  hd_t *hd;
  devtree_t *dt, *next;

  /*
   * first, remove all references in the current device tree
   * (refs in hd_old can remain)
   */
  for(hd = hd_data->hd; hd; hd = hd->next) {
    if(hd->detail && hd->detail->type == hd_detail_devtree) {
      hd->detail = free_mem(hd->detail);
    }
  }

  for(dt = hd_data->devtree; dt; dt = next) {
    next = dt->next;

    free_mem(dt->path);
    free_mem(dt->filename);
    free_mem(dt->name);
    free_mem(dt->model);
    free_mem(dt->device_type);
    free_mem(dt->compatible);
    free_mem(dt->edid);

    free_mem(dt);
  }

  return hd_data->devtree = NULL;
}


void test_read_block0_open(void *arg)
{
  open((char *) arg, O_RDONLY);
}

unsigned char *read_block0(hd_data_t *hd_data, char *dev, int *timeout)
{
  int fd, len, buf_size = 512, k, sel;
  unsigned char *buf = NULL;
  struct timeval to;
  fd_set set, set0;

  if(hd_timeout(test_read_block0_open, dev, *timeout) > 0) {
    ADD2LOG("  read_block0: open(%s) timed out\n", dev);
    *timeout = -1;
    fd = -2;
  }
  else {
    fd = open(dev, O_RDONLY);
    if(fd < 0) ADD2LOG("  read_block0: open(%s) failed\n", dev);
  }
  if(fd >= 0) {
    buf = new_mem(buf_size);
    len = k = 0;

    FD_ZERO(&set0);
    FD_SET(fd, &set0);

    to.tv_sec = *timeout; to.tv_usec = 0;
    for(;;) {
      set = set0;
      if((sel = select(fd + 1, &set, NULL, NULL, &to)) > 0) {
        if((k = read(fd, buf + len, buf_size - len)) > 0) len += k;
        ADD2LOG("  read_block0: %d bytes (%ds, %dus)\n", k, (int) to.tv_sec, (int) to.tv_usec);
        if(k <= 0 || buf_size == len) break;
      }
      if(sel == 0) {
        *timeout = -2; break;
      }
    }

    if(k < 0) {
      ADD2LOG("  read_block0: read error(%s, %d, %d): errno %d\n", dev, len, buf_size - len, errno);
      buf = free_mem(buf);
    }
    close(fd);
  }

  return buf;
}


void get_kernel_version(hd_data_t *hd_data)
{
  unsigned u1, u2;
  str_list_t *sl;

  if(hd_data->kernel_version) return;

  hd_data->kernel_version = KERNEL_24;

  sl = read_file(PROC_VERSION, 0, 1);

  if(!sl || !sl->str) return;

  if(sscanf(sl->str, "Linux version %u.%u.", &u1, &u2) == 2) {
    if(hd_data->debug) {
      ADD2LOG("kernel version is %u.%u\n", u1, u2);
    }
    u1 = (u1 << 16) + (u2 << 8);

    if(u1 <= KERNEL_22) hd_data->kernel_version = KERNEL_22;
  }

  free_str_list(sl);
}


char *vend_id2str(unsigned vend)
{
  static char buf[32];
  char *s;

  *(s = buf) = 0;

  if(ID_TAG(vend) == TAG_EISA) {
    strcpy(s, eisa_vendor_str(vend));
  }
  else {
    if(ID_TAG(vend) == TAG_USB) *s++ = 'u', *s = 0;
    if(ID_TAG(vend) == TAG_SPECIAL) *s++ = 's', *s = 0;
    sprintf(s, "%04x", ID_VALUE(vend));
  }

  return buf;
}



void assign_hw_class(hd_data_t *hd_data, hd_t *hd)
{
  int sc;		/* compare sub_class too */
  unsigned base_class, sub_class;
  hd_hw_item_t item;
  int (*test_func)(hd_data_t *, hd_t *);

  if(!hd) return;

  if(!hd->hw_class) {		/* skip if we've already done it */
    for(item = 1; item < hw_all; item++) {

      test_func = NULL;

      sc = 0;
      sub_class = 0;

      base_class = -1;
      switch(item) {
        case hw_cdrom:
          base_class = bc_storage_device;
          sub_class = sc_sdev_cdrom;
          sc = 1;
          break;

        case hw_floppy:
          base_class = bc_storage_device;
          sub_class = sc_sdev_floppy;
          sc = 1;
          break;

        case hw_disk:
          base_class = bc_storage_device;
          sub_class = sc_sdev_disk;
          sc = 1;
          break;

        case hw_network:
          base_class = bc_network_interface;
          break;

        case hw_display:
          base_class = bc_display;
          break;

        case hw_monitor:
          base_class = bc_monitor;
          break;

        case hw_mouse:
          base_class = bc_mouse;
          break;

        case hw_joystick:
          base_class = bc_joystick;
          break;

        case hw_keyboard:
          base_class = bc_keyboard;
          break;

        case hw_camera:
          base_class = bc_camera;
          break;

        case hw_framebuffer:
          base_class = bc_framebuffer;
          break;

        case hw_chipcard:
          base_class = bc_chipcard;
          break;

        case hw_sound:
          base_class = bc_multimedia;
          sub_class = sc_multi_audio;
          sc = 1;
          break;

        case hw_isdn:
          base_class = bc_isdn;
          break;

        case hw_modem:
          base_class = bc_modem;
          break;

        case hw_storage_ctrl:
          base_class = bc_storage;
          break;

        case hw_network_ctrl:
          base_class = bc_network;
          break;

        case hw_printer:
          base_class = bc_printer;
          break;

        case hw_tv:
          base_class = bc_tv;
          break;

        case hw_dvb:
          base_class = bc_dvb;
          break;

        case hw_scanner:
          base_class = bc_scanner;
          break;

        case hw_braille:
          base_class = bc_braille;
          break;

        case hw_sys:
          base_class = bc_internal;
          sub_class = sc_int_sys;
          sc = 1;
          break;

        case hw_cpu:
          base_class = bc_internal;
          sub_class = sc_int_cpu;
          sc = 1;
          break;

        case hw_bios:
          base_class = bc_internal;
          sub_class = sc_int_bios;
          sc = 1;
          break;

        case hw_usb_ctrl:
          base_class = bc_serial;
          sub_class = sc_ser_usb;
          sc = 1;
          break;

        case hw_bridge:
          base_class = bc_bridge;
          break;

        case hw_hub:
          base_class = bc_hub;
          break;

        case hw_memory:
          base_class = bc_internal;
          sub_class = sc_int_main_mem;
          sc = 1;
          break;

        case hw_ieee1394_ctrl:
          base_class = bc_serial;
          sub_class = sc_ser_fire;
          sc = 1;
          break;

        case hw_pcmcia_ctrl:
          test_func = is_pcmcia_ctrl;
          break;

        case hw_unknown:
        case hw_all:
        case hw_manual:		/* special */

        /* bus types */
        case hw_usb:
        case hw_pci:
        case hw_isapnp:
        case hw_scsi:
        case hw_ide:

        case hw_pcmcia:		/* not handled */
        case hw_ieee1394:	/* not handled */
        case hw_hotplug:	/* not handled */
        case hw_hotplug_ctrl:	/* not handled */
        case hw_zip:		/* not handled */
        case hw_partition:	/* not handled */
          break;
      }

      if(test_func) {
        if(test_func(hd_data, hd)) {
          hd->hw_class = item;
          break;
        }
      }
      else if(
        (
          hd->base_class.id == base_class &&
          (sc == 0 || hd->sub_class.id == sub_class)
        )
        ||
        ( /* list other display adapters, too */
          base_class == bc_display &&
          hd->base_class.id == bc_multimedia &&
          hd->sub_class.id == sc_multi_video
        )
        ||
        ( /* make i2o storage controllers */
          item == hw_storage_ctrl && hd->base_class.id == bc_i2o
        )
      ) {

        /* ISA-PnP sound cards: just one entry per card */
        if(
          item == hw_sound &&
          hd->bus.id == bus_isa &&
          hd->is.isapnp &&
          hd->func
        ) continue;


        hd->hw_class = item;
        break;
      }
    }

    if(!hd->hw_class) hd->hw_class = hw_unknown;
  }

  if(!hd->hw_class2) {
    if(hd->bus.id == bus_usb) {
      hd->hw_class2 = hw_usb;
    }
    else if(hd->bus.id == bus_pci) {
      hd->hw_class2 = hw_pci;
    }
    else if(hd->bus.id == bus_scsi) {
      hd->hw_class2 = hw_scsi;
    }
    else if(hd->bus.id == bus_ide) {
      hd->hw_class2 = hw_ide;
    }
    else if(hd->bus.id == bus_isa && hd->is.isapnp) {
      hd->hw_class2 = hw_isapnp;
    }
  }

  if(!hd->hw_class3) {
    if(hd->usb_guid) {
#if 0
      if(hd->bus.id == bus_usb) {
        hd->hw_class3 = hw_scsi;
      }
      else
#endif
      if(hd->bus.id == bus_scsi) {
        hd->hw_class3 = hw_usb;
      }
    }
  }
}


#ifndef LIBHD_TINY
void short_vendor(char *vendor)
{
  static char *remove[] = {
    ".", ",", "-", "&", " inc", "corporation", " corp", " system",
    " systems", "technology", "technologies", "multimedia", "communications",
    "computer", " ltd", "(formerly ncr)", " group", " labs", "research",
    "equipment", " ag", "personal", " canada", "data", "products",
    " america", " co", " of", "solutions", " as", "publishing", "(old)",
    " usa", " gmbh", "electronic", "components", "(matsushita)", " ab",
    " pte", " north", " japan", "limited", "microcomputer", " kg",
    "incorporated", "semiconductor", "graphics"
  };
  int i, j;
  int len, len1, len2;

  if(!vendor) return;

  len2 = strlen(vendor);

  if(!len2) return;

  do {
    len = len2;
    for(i = 0; i < sizeof remove / sizeof *remove; i++) {
      len1 = strlen(remove[i]);
      if(len > len1 && !strcasecmp(vendor + len - len1, remove[i])) {
        vendor[j = len - len1] = 0;
        for(j--; j >= 0; vendor[j--] = 0) {
          if(!isspace(vendor[j])) break;
        }
      }
    }
    len2 = strlen(vendor);
  } while(len2 != len);

}


void create_model_name(hd_data_t *hd_data, hd_t *hd)
{
  char *vend, *dev;
  char *compat, *dev_class, *hw_class;
  char *part1, *part2;
  cpu_info_t *ct;

  /* early out */
  if(!hd || hd->model) return;

  part1 = part2 = NULL;

  vend = dev = compat = dev_class = hw_class = NULL;

  if(
    hd->hw_class == hw_cpu &&
    hd->detail &&
    hd->detail->type == hd_detail_cpu &&
    (ct = hd->detail->cpu.data) &&
    ct->model_name
  ) {
    /* cpu entry */

    part1 = new_str(ct->model_name);
    if(ct->clock) str_printf(&part1, -1, ", %u MHz", ct->clock);
  }
  else {
    /* normal entry */

    vend = new_str(hd->sub_vendor.name);

    dev = new_str(hd->sub_device.name);

    if(!vend) vend = new_str(hd->vendor.name);

    if(!dev) dev = new_str(hd->device.name);

    if(dev) {
      if(vend) {
        part1 = vend; part2 = dev;
      }
      else {
        part1 = dev;
      }
    }

    if(!part1 && !part2) {
      compat = new_str(hd->compat_device.name);

      dev_class = new_str(hd->sub_class.name ?: hd->base_class.name);

      hw_class = new_str(hd_hw_item_name(hd->hw_class));

      if(vend) {
        if(compat) {
          part1 = vend; part2 = compat;
        }
        else if(dev_class) {
          part1 = vend; part2 = dev_class;
        }
      }
      else {
        if(compat && dev_class) {
          part1 = compat; part2 = dev_class;
        }
        else if(compat) {
          part1 = compat;
        }
        else if(dev_class) {
          part1 = dev_class;
          if(hw_class && !strchr(part1, ' ') && strchr(hw_class, ' ')) {
            part2 = hw_class;
          }
        }
      }
    }
  }

  if(part1 && part2) {
    short_vendor(part1);
  }

  if(part1 && !strcasecmp(part1, "unknown")) {
    part1 = part2;
    part2 = NULL;
  }

  if(part1 && part2) {
    /* maybe the vendor name is already part of the device name... */
    if(strstr(part2, part1) || !strcasecmp(part1, part2)) {
      part1 = part2;
      part2 = NULL;
    }
  }

  if(!part1 && !part2 && hw_class) {
    str_printf(&part1, 0, "unknown %s", hw_class);
    if(strchr(hw_class, ' ')) {
      str_printf(&part1, -1, " hardware");
    }
  }

  str_printf(&hd->model, 0, "%s%s%s", part1, part2 ? " " : "", part2 ? part2 : "");

  free_mem(vend);
  free_mem(dev);
  free_mem(compat);
  free_mem(dev_class);
  free_mem(hw_class);
}


int hd_change_status(const char *id, hd_status_t status, const char *config_string)
{
  hd_data_t *hd_data;
  hd_manual_t *entry;
  int i;

  hd_data = new_mem(sizeof *hd_data);

  entry = hd_manual_read_entry(hd_data, (char *) id);

  if(!entry || status.invalid) return 1;

  if(status.configured) entry->status.configured = status.configured;
  if(status.available) entry->status.available = status.available;
  if(status.needed) entry->status.needed = status.needed;
  entry->status.invalid = status.invalid;

  if(config_string) {
    free_mem(entry->config_string);
    entry->config_string = new_str(config_string);
  }

  i = hd_manual_write_entry(hd_data, entry);
  
  hd_free_manual(entry);

  hd_free_hd_data(hd_data);

  free_mem(hd_data);

  return i;
}

#endif		/* !defined(LIBHD_TINY) */


void hd_getdisksize(hd_data_t *hd_data, char *dev, int fd, hd_res_t **geo, hd_res_t **size)
{
  hd_res_t *res;
  struct hd_geometry geo_s;
  struct hd_big_geometry big_geo_s;
  unsigned long secs;
  unsigned sec_size;
  int close_fd = 0;

  *geo = *size = NULL;

  ADD2LOG("  dev = >%s<, fd = %d\n", dev, fd);

  if(fd < 0) {
    if(!dev) return;
    fd = open(dev, O_RDONLY | O_NONBLOCK);
    close_fd = 1;
    if(fd < 0) return;
  }

  ADD2LOG("  open ok, fd = %d\n", fd);

  if(!ioctl(fd, HDIO_GETGEO_BIG, &big_geo_s)) {
    if(dev) ADD2LOG("%s: ioctl(big geo) ok\n", dev);
    res = add_res_entry(geo, new_mem(sizeof *res));
    res->disk_geo.type = res_disk_geo;
    res->disk_geo.cyls = big_geo_s.cylinders;
    res->disk_geo.heads = big_geo_s.heads;
    res->disk_geo.sectors = big_geo_s.sectors;
    res->disk_geo.logical = 1;
  }
  else {
    ADD2LOG("  big geo failed: %s\n", strerror(errno));
    if(!ioctl(fd, HDIO_GETGEO, &geo_s)) {
      if(dev) ADD2LOG("%s: ioctl(geo) ok\n", dev);
      res = add_res_entry(geo, new_mem(sizeof *res));
      res->disk_geo.type = res_disk_geo;
      res->disk_geo.cyls = geo_s.cylinders;
      res->disk_geo.heads = geo_s.heads;
      res->disk_geo.sectors = geo_s.sectors;
      res->disk_geo.logical = 1;
    }
    else {
      ADD2LOG("  geo failed: %s\n", strerror(errno));
    }
  }

  if(!ioctl(fd, BLKGETSIZE, &secs)) {
    if(dev) ADD2LOG("%s: ioctl(disk size) ok\n", dev);
  }
  else {
    secs = 0;
    if(*geo) secs = (*geo)->disk_geo.cyls * (*geo)->disk_geo.heads * (*geo)->disk_geo.sectors;
  }

  if(secs) {
    res = add_res_entry(size, new_mem(sizeof *res));
    res->size.type = res_size;
    res->size.unit = size_unit_sectors;
    res->size.val1 = secs;
    res->size.val2 = 512;
    if(!ioctl(fd, BLKSSZGET, &sec_size)) {
      if(dev) ADD2LOG("%s: ioctl(block size) ok\n", dev);
      if(sec_size) res->size.val2 = sec_size;
    }
  }

  ADD2LOG("  geo = %p, size = %p\n", *geo, *size);

  if(close_fd) close(fd);
}


str_list_t *hd_split(char del, char *str)
{
  char *t, *s;
  str_list_t *sl = NULL;

  if(!str) return NULL;

  for(s = str = new_str(str); (t = strchr(s, del)); s = t + 1) {
    *t = 0;
    add_str_list(&sl, s);
  }
  add_str_list(&sl, s);

  free_mem(str);

  return sl;
}


char *hd_join(char del, str_list_t *str)
{
  char *s, t[2];
  str_list_t *str0;
  int len = 0;

  for(str0 = str; str0; str0 = str0->next) {
    len += strlen(str0->str) + 1;
  }

  if(!len) return NULL;

  s = new_mem(len);

  t[0] = del; t[1] = 0;

  for(; str; str = str->next) {
    strcat(s, str->str);
    if(str->next) strcat(s, t);
  }

  return s;
}


/*
 * cf. pcmcia-cs-*:cardmgr/probe.c
 */
int is_pcmcia_ctrl(hd_data_t *hd_data, hd_t *hd)
{
  int i;
  static unsigned ids[][2] = {
    { 0x1013, 0x1100 },
    { 0x1013, 0x1110 },
    { 0x10b3, 0xb106 },
    { 0x1180, 0x0465 },
    { 0x1180, 0x0466 },
    { 0x1180, 0x0475 },
    { 0x1180, 0x0476 },
    { 0x1180, 0x0478 },
    { 0x104c, 0xac12 },
    { 0x104c, 0xac13 },
    { 0x104c, 0xac15 },
    { 0x104c, 0xac16 },
    { 0x104c, 0xac17 },
    { 0x104c, 0xac19 },
    { 0x104c, 0xac1a },
    { 0x104c, 0xac1d },
    { 0x104c, 0xac1f },
    { 0x104c, 0xac1b },
    { 0x104c, 0xac1c },
    { 0x104c, 0xac1e },
    { 0x104c, 0xac51 },
    { 0x1217, 0x6729 },
    { 0x1217, 0x673a },
    { 0x1217, 0x6832 },
    { 0x1217, 0x6836 },
    { 0x1217, 0x6872 },
    { 0x1179, 0x0603 },
    { 0x1179, 0x060a },
    { 0x1179, 0x060f },
    { 0x1179, 0x0617 },
    { 0x119b, 0x1221 },
    { 0x8086, 0x1221 }
  };

  if(!hd) return 0;

  if(
     hd->base_class.id == bc_bridge &&
    (hd->sub_class.id == sc_bridge_pcmcia || hd->sub_class.id == sc_bridge_cardbus)
  ) return 1;

  /* just in case... */
  if(hd->bus.id == bus_pci) {
    for(i = 0; i < sizeof ids / sizeof *ids; i++) {
      if(
        ID_VALUE(hd->vendor.id) == ids[i][0] &&
        ID_VALUE(hd->device.id) == ids[i][1]
      ) return 1;
    }
  }

  return 0;
}

