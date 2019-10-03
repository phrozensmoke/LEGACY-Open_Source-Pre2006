#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include "vbios.h"

#include "hd.h"
#include "hd_int.h"
#include "bios.h"

static unsigned segofs2addr(unsigned char *segofs);
static unsigned get_data(unsigned char *buf, unsigned buf_size, unsigned addr);
static void read_vbe_info(hd_data_t *hd_data, vbe_info_t *vbe, unsigned char *vbeinfo);

static hd_data_t *log_hd_data;
void log_err(char *format, ...) __attribute__ ((format (printf, 1, 2)));

void get_vbe_info(hd_data_t *hd_data, vbe_info_t *vbe)
{
  int i;
  unsigned char vbeinfo[0x200];
  int ax, bx, cx;

  log_hd_data = hd_data;

  if(InitInt10()) {
    ADD2LOG("VBE: Could not init Int10\n");
    return;
  }
  memset(vbeinfo, 0, sizeof vbeinfo);
  strcpy(vbeinfo, "VBE2");

  PROGRESS(4, 1, "vbe info");

  ax = 0x4f00; bx = 0; cx = 0;
  i = CallInt10(&ax, &bx, &cx, vbeinfo, sizeof vbeinfo) & 0xffff;

  if(i != 0x4f) {
    ADD2LOG("VBE: Error (0x4f00): 0x%04x\n", i);
    FreeInt10();
    return;
  }

  PROGRESS(4, 2, "mode info");

  read_vbe_info(hd_data, vbe, vbeinfo);

  PROGRESS(4, 3, "ddc info");

  memset(vbeinfo, 0, sizeof vbeinfo);
  strcpy(vbeinfo, "VBE2");
  ax = 0x4f15; bx = 1; cx = 0;
  i = CallInt10(&ax, &bx, &cx, vbeinfo, sizeof vbeinfo) & 0xffff;

  if(i != 0x4f) {
    ADD2LOG("Error (0x4f15): 0x%04x\n", i);
  } else {
    memcpy(vbe->ddc, vbeinfo, sizeof vbe->ddc);

    ADD2LOG("edid record:\n");
    for(i = 0; i < sizeof vbe->ddc; i += 0x10) {
      ADD2LOG("  ");
      hexdump(&hd_data->log, 1, 0x10, vbe->ddc + i);
      ADD2LOG("\n");
    }
  }

  if(hd_probe_feature(hd_data, pr_bios_vbe2)) {
    ax = 0x4f03; bx = 0; cx = 0;
    i = CallInt10(&ax, &bx, &cx, vbeinfo, sizeof vbeinfo) & 0xffff;

    if(i != 0x4f) {
      ADD2LOG("Error (0x4f03): 0x%04x\n", i);
    } else {
      vbe->current_mode = bx;
    }
  }

  FreeInt10();
}


unsigned segofs2addr(unsigned char *segofs)
{
  return segofs[0] + (segofs[1] << 8) + (segofs[2] << 4)+ (segofs[3] << 12);
}


unsigned get_data(unsigned char *buf, unsigned buf_size, unsigned addr)
{
  unsigned bufferaddr = 0x7e00;
  unsigned len;

  *buf = 0;
  len = 0;

  if(addr >= bufferaddr && addr < bufferaddr + 0x200) {
    len = bufferaddr + 0x200 - addr;
    if(len >= buf_size) len = buf_size - 1;
    memcpy(buf, (void *) addr, len);
  }
  else if(addr >= 0x0c0000 && addr < 0x100000) {
    len = 0x100000 - addr;
    if(len >= buf_size) len = buf_size - 1;
    memcpy(buf, (void *) addr, len);
  }

  buf[len] = 0;

  return len;
}


void read_vbe_info(hd_data_t *hd_data, vbe_info_t *vbe, unsigned char *v)
{
  unsigned char tmp[1024], s[64];
  unsigned i, l, u;
  unsigned modelist[0x100];
  unsigned bpp, res_bpp, fb, clock;
  vbe_mode_info_t *mi;
  int ax, bx, cx;

  vbe->ok = 1;

  vbe->version = v[0x04] + (v[0x05] << 8);
  vbe->oem_version = v[0x14] + (v[0x15] << 8);
  vbe->memory = (v[0x12] + (v[0x13] << 8)) << 16;

  ADD2LOG(
    "version = %u.%u, oem version = %u.%u\n",
    vbe->version >> 8, vbe->version & 0xff, vbe->oem_version >> 8, vbe->oem_version & 0xff
  );

  ADD2LOG("memory = %uk\n", vbe->memory >> 10);

  l = get_data(tmp, sizeof tmp, u = segofs2addr(v + 0x06));
  vbe->oem_name = canon_str(tmp, l);
  ADD2LOG("oem name [0x%05x] = \"%s\"\n", u, vbe->oem_name);

  l = get_data(tmp, sizeof tmp, u = segofs2addr(v + 0x16));
  vbe->vendor_name = canon_str(tmp, l);
  ADD2LOG("vendor name [0x%05x] = \"%s\"\n", u, vbe->vendor_name);

  l = get_data(tmp, sizeof tmp, u = segofs2addr(v + 0x1a));
  vbe->product_name = canon_str(tmp, l);
  ADD2LOG("product name [0x%05x] = \"%s\"\n", u, vbe->product_name);

  l = get_data(tmp, sizeof tmp, u = segofs2addr(v + 0x1e));
  vbe->product_revision = canon_str(tmp, l);
  ADD2LOG("product revision [0x%05x] = \"%s\"\n", u, vbe->product_revision);

  l = get_data(tmp, sizeof tmp, u = segofs2addr(v + 0x0e)) >> 1;

  for(i = vbe->modes = 0; i < l && i < sizeof modelist / sizeof *modelist; i++) {
    u = tmp[2 * i] + (tmp[2 * i + 1] << 8);
    if(u != 0xffff)
      modelist[vbe->modes++] = u;
    else
      break;
  }

  ADD2LOG("%u video modes:\n", vbe->modes);

  vbe->mode = new_mem(vbe->modes * sizeof *vbe->mode);

  if(!vbe->mode) return;

  for(i = 0; i < vbe->modes; i++) {

    mi = vbe->mode + i;

    mi->number =  modelist[i];
    
    ax = 0x4f01; bx = 0; cx = modelist[i];
    l = CallInt10(&ax, &bx, &cx, tmp, sizeof tmp) & 0xffff;

    if(l != 0x4f) {
      ADD2LOG("0x%04x: no mode info\n", modelist[i]);
      continue;
    }

    mi->attributes = tmp[0x00] + (tmp[0x01] << 8);

    mi->width = tmp[0x12] + (tmp[0x13] << 8);
    mi->height = tmp[0x14] + (tmp[0x15] << 8);
    mi->bytes_p_line = tmp[0x10] + (tmp[0x11] << 8);

    mi->win_A_start = (tmp[0x08] + (tmp[0x09] << 8)) << 4;
    mi->win_B_start = (tmp[0x0a] + (tmp[0x0b] << 8)) << 4;

    mi->win_A_attr = tmp[0x02];
    mi->win_B_attr = tmp[0x03];

    mi->win_gran = (tmp[0x04] + (tmp[0x05] << 8)) << 10;
    mi->win_size = (tmp[0x06] + (tmp[0x07] << 8)) << 10;

    bpp = res_bpp = 0;
    switch(tmp[0x1b]) {
      case 0:
        bpp = -1;
        break;

      case 1:
        bpp = 2;
        break;

      case 2:
        bpp = 1;
        break;

      case 3:
        bpp = 4;
        break;

      case 4:
        bpp = 8;
        break;

      case 6:
        bpp = tmp[0x19] - tmp[0x25];
        res_bpp = tmp[0x25];
    }

    fb = 0;
    if(vbe->version >= 0x0200) {
      mi->fb_start = tmp[0x28] + (tmp[0x29] << 8) + (tmp[0x2a] << 16) + (tmp[0x2b] << 24);
    }

    clock = 0;
    if(vbe->version >= 0x0300) {
      mi->pixel_clock = tmp[0x3e] + (tmp[0x3f] << 8) + (tmp[0x40] << 16) + (tmp[0x41] << 24);
    }

    mi->pixel_size = bpp;

    if(bpp == -1) {
      ADD2LOG("  0x%04x[%02x]: %ux%u, text\n", mi->number, mi->attributes, mi->width, mi->height);
    }
    else {
      if(
        (mi->attributes & 1) &&		/* mode is supported */
        mi->fb_start
      ) {
        if(!vbe->fb_start) vbe->fb_start = mi->fb_start;
      }
      *s = 0;
      if(res_bpp) sprintf(s, "+%u", res_bpp);
      ADD2LOG(
        "  0x%04x[%02x]: %ux%u+%u, %u%s bpp",
        mi->number, mi->attributes, mi->width, mi->height, mi->bytes_p_line, mi->pixel_size, s
      );

      if(mi->pixel_clock) ADD2LOG(", max. %u MHz", mi->pixel_clock/1000000);

      if(mi->fb_start) ADD2LOG(", fb: 0x%08x", mi->fb_start);

      ADD2LOG(", %04x.%x", mi->win_A_start, mi->win_A_attr);

      if(mi->win_B_start || mi->win_B_attr) ADD2LOG("/%04x.%x", mi->win_B_start, mi->win_B_attr);

      ADD2LOG(": %uk", mi->win_size >> 10);

      if(mi->win_gran != mi->win_size) ADD2LOG("/%uk", mi->win_gran >> 10);

      ADD2LOG("\n");
    }
  }

}

void log_err(char *format, ...)
{
  va_list args;
  char buf[1024];

  va_start(args, format);
  vsnprintf(buf, sizeof buf, format, args);
  str_printf(&log_hd_data->log, -2, "%s", buf);
  va_end(args);
}

