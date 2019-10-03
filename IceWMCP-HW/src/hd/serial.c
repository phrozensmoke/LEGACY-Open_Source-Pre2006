#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "hd.h"
#include "hd_int.h"
#include "serial.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * serial interface
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */


static char *ser_names[] = {
  "8250", "16450", "16550", "16650", "16750", "16850", "16950"
};

static void get_serial_info(hd_data_t *hd_data);
static serial_t *add_serial_entry(serial_t **ser, serial_t *new_ser);
static void dump_serial_data(hd_data_t *hd_data);

void hd_scan_serial(hd_data_t *hd_data)
{
  hd_t *hd;
  serial_t *ser, *next;
  hd_res_t *res;
  int i;
  char *s, *skip_dev[2] = { NULL, NULL }, buf[4];

  if(!hd_probe_feature(hd_data, pr_serial)) return;

  hd_data->module = mod_serial;

  /* some clean-up */
  remove_hd_entries(hd_data);
  hd_data->serial = NULL;

  PROGRESS(1, 0, "read info");

  get_serial_info(hd_data);
  if((hd_data->debug & HD_DEB_SERIAL)) dump_serial_data(hd_data);

  for(i = 0; i < sizeof skip_dev / sizeof *skip_dev; i++) {
    s = get_cmdline(hd_data, i == 0 ? "yast2ser" : "console");
    if(
      s && *s &&
      sscanf(s, "tty%3[^,]", buf) == 1
    ) {
      if(buf[1] == 0) {
        switch(*buf) {
          case 'a': strcpy(buf, "S0"); break;
          case 'b': strcpy(buf, "S1"); break;
        }
      }
      str_printf(&skip_dev[i], 0, "/dev/tty%s", buf);
    }
    free_mem(s);
  }

  PROGRESS(2, 0, "build list");

  for(ser = hd_data->serial; ser; ser = ser->next) {
    hd = add_hd_entry(hd_data, __LINE__, 0);
    hd->base_class.id = bc_comm;
    hd->sub_class.id = sc_com_ser;
    hd->prog_if.id = 0x80;
    for(i = 0; i < sizeof ser_names / sizeof *ser_names; i++) {
      if(strstr(ser->name, ser_names[i])) hd->prog_if.id = i;
    }
    hd->device.name = new_str(ser->name);
    hd->func = ser->line;
    str_printf(&hd->unix_dev_name, 0, "/dev/ttyS%u", ser->line);
    for(i = 0; i < sizeof skip_dev / sizeof *skip_dev; i++) {
      if(skip_dev[i] && !strcmp(skip_dev[i], hd->unix_dev_name)) {
        hd->tag.ser_skip = 1;
        break;
      }
    }
    if(ser->device) {
      if(strstr(ser->device, "modem-printer")) {
        hd->tag.ser_device = 1;
      }
      else if(strstr(ser->device, "infrared")) {
         hd->tag.ser_device = 2;
      }
      else if(strstr(ser->device, "modem")) {
         hd->tag.ser_device = 3;
      }
    }
    if(ser->baud) {
      res = add_res_entry(&hd->res, new_mem(sizeof *res));
      res->baud.type = res_baud;
      res->baud.speed = ser->baud;
    }
    res = add_res_entry(&hd->res, new_mem(sizeof *res));
    res->io.type = res_io;
    res->io.enabled = 1;
    res->io.base = ser->port;
    res->io.access = acc_rw;

    res = add_res_entry(&hd->res, new_mem(sizeof *res));
    res->irq.type = res_irq;
    res->irq.enabled = 1;
    res->irq.base = ser->irq;
  }  

  for(ser = hd_data->serial; ser; ser = next) {
    next = ser->next;

    free_mem(ser->name);
    free_mem(ser->device);
    free_mem(ser);
  }
  hd_data->serial = NULL;

#if 0
  if(hd_module_is_active(hd_data, "irda")) {
    hd = add_hd_entry(hd_data, __LINE__, 0); 
    hd->base_class.id = bc_comm;
    hd->sub_class.id = sc_com_ser;
    hd->prog_if.id = 0x80;
    hd->device.name = new_str("IrDA Serial");
    hd->unix_dev_name = new_str("/dev/ircomm0");
  }
#endif
}

void get_serial_info(hd_data_t *hd_data)
{
  char buf[64];
  unsigned u0, u1, u2;
#if !defined(__PPC__)
  unsigned u3;
#endif
  int i;
  str_list_t *sl, *sl0;
  serial_t *ser;

#if !defined(__PPC__)
  /*
   * Max. 44 serial lines at the moment; the serial proc interface is
   * somewhat buggy at the moment (2.2.13), hence the explicit 44 lines
   * limit. That may be dropped later.
   */
  sl0 = read_file(PROC_DRIVER_SERIAL, 1, 44);

  // ########## FIX !!!!!!!! ########
  if(sl0) {
    for(sl = sl0; sl; sl = sl->next) {
      i = 0;
      if(
        sscanf(sl->str, "%u: uart:%31s port:%x irq:%u baud:%u", &u0, buf, &u1, &u2, &u3) == 5 ||
        (i = 1, sscanf(sl->str, "%u: uart:%31s port:%x irq:%u tx:%u", &u0, buf, &u1, &u2, &u3) == 5)
      ) {
        /*
         * The 'baud' or 'tx' entries are only present for real interfaces.
         */
        ser = add_serial_entry(&hd_data->serial, new_mem(sizeof *ser));
        ser->line = u0;
        ser->port = u1;
        ser->irq = u2;
        if(!i) ser->baud = u3;
        ser->name = new_str(buf);
      }
    }

    if((hd_data->debug & HD_DEB_SERIAL)) {
      /* log just the first 16 entries */
      ADD2LOG("----- "PROC_DRIVER_SERIAL" -----\n");
      for(sl = sl0, i = 16; sl && i--; sl = sl->next) {
        ADD2LOG("  %s", sl->str);
      }
      ADD2LOG("----- "PROC_DRIVER_SERIAL" end -----\n");
    }
  }
#endif	/* !defined(__PPC__) */

#if defined(__PPC__)
  sl0 = read_file(PROC_DRIVER_MACSERIAL, 1, 0);

  if(sl0) {
    for(sl = sl0; sl; sl = sl->next) {
      if(
        (i = sscanf(sl->str, "%u: port:%x irq:%u con:%63[^\n]", &u0, &u1, &u2, buf)) >= 3
      ) {
        ser = add_serial_entry(&hd_data->serial, new_mem(sizeof *ser));
        ser->line = u0;
        ser->port = u1;
        ser->irq = u2;
        ser->name = new_str("SCC");
        if(i == 4) ser->device = new_str(buf);
      }
    }

    if((hd_data->debug & HD_DEB_SERIAL)) {
      /* log just the first 16 entries */
      ADD2LOG("----- "PROC_DRIVER_MACSERIAL" -----\n");
      for(sl = sl0, i = 16; sl && i--; sl = sl->next) {
        ADD2LOG("  %s", sl->str);
      }
      ADD2LOG("----- "PROC_DRIVER_MACSERIAL" end -----\n");
    }
  }
#endif	/* defined(__PPC__) */


  free_str_list(sl0);
}

serial_t *add_serial_entry(serial_t **ser, serial_t *new_ser)
{
  while(*ser) ser = &(*ser)->next;
  return *ser = new_ser;
}

void dump_serial_data(hd_data_t *hd_data)
{
  serial_t *ser;

  ADD2LOG("----- serial info -----\n");
  for(ser = hd_data->serial; ser; ser = ser->next) {
    ADD2LOG(
      "  uart %s, line %d, port 0x%03x, irq %d, baud %d\n",
      ser->name, ser->line, ser->port, ser->irq, ser->baud
    );
  }
  ADD2LOG("----- serial info end -----\n");
}

