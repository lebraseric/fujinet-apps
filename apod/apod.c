/*
  apod.c

  Astronomy Picture of the Day (APOD) viewer client
  for Ataris using #Fujinet.

  See the APOD web app (server)

  By Bill Kendrick <bill@newbreedsoftware.com>
  2021-03-27 - 2021-03-30
*/

#define PLACEHOLDER

#include <stdio.h>
#include <atari.h>
#include <peekpoke.h>
#include "nsio.h"

#define DLIST_SIZE 1024
extern unsigned char dlist_mem[];
extern unsigned char scr_mem[];

/**
 * Simple text rendering onto screen memory
 */
void myprint(unsigned char x, unsigned char y, char * str) {
  int pos, i;
  unsigned char c;
  unsigned char * SC;

  SC = (unsigned char *) PEEKW(88);

  pos = y * 20 + x;
  for (i = 0; str[i] != '\0'; i++) {
    c = str[i];

    if (c < 32) {
      c = c + 64;
    } else if (c < 96) {
      c = c - 32;
    }

    SC[pos + i] = c;
  }
}


void dlist_setup(unsigned char antic_mode) {
  unsigned char i;
  unsigned int gfx_ptr, dlist_idx;

  OS.sdmctl = 0;

  dlist_idx = 0;

  dlist_mem[dlist_idx++] = DL_BLK8;
  dlist_mem[dlist_idx++] = DL_BLK8;
  dlist_mem[dlist_idx++] = DL_BLK8;

  gfx_ptr = (unsigned int) scr_mem;
  for (i = 0; i < 192; i++) {
    dlist_mem[dlist_idx++] = DL_LMS(antic_mode);
    dlist_mem[dlist_idx++] = (gfx_ptr & 255);
    dlist_mem[dlist_idx++] = (gfx_ptr >> 8);
    gfx_ptr += 40;
  }

  dlist_mem[dlist_idx++] = DL_JVB;
  dlist_mem[dlist_idx++] = ((unsigned int) dlist_mem & 255);
  dlist_mem[dlist_idx++] = ((unsigned int) dlist_mem >> 8);

  OS.sdlst = dlist_mem;
  OS.sdmctl = DMACTL_PLAYFIELD_NORMAL | DMACTL_DMA_FETCH;
}

unsigned char active_dlist;
unsigned char dlist_hi;
void * OLDVEC;

unsigned char rgb_ctr;
unsigned char rgb_table[3] = { 0x40, 0xC0, 0x80 };

#pragma optimize (push, off)
void VBLANKD(void) {
  asm("lda %v", dlist_hi);
  asm("adc %v", active_dlist);
  asm("sta $d403");

  asm("lda %v", active_dlist);
  asm("adc #4");
  asm("cmp #12");
  asm("bcc %g", __vbi_dlist_done);

  asm("lda #0");

__vbi_dlist_done:
  asm("sta %v", active_dlist);

  asm("ldx %v", rgb_ctr);
  asm("inx");
  asm("cmp #4");
  asm("bcc %g", __vbi_done);

  asm("ldx #0");

__vbi_done:
  asm("stx %v", rgb_ctr);

  asm("jmp (%v)", OLDVEC);
}

void mySETVBV(void * Addr)
{
  active_dlist = 0;
  dlist_hi = (unsigned char) (((unsigned int) dlist_mem) >> 8);

  OS.critic = 1;
  OS.vvblkd = Addr;
  OS.critic = 0;

  ANTIC.nmien = NMIEN_VBI;
}

void dli(void)
{
  asm("pha");
  asm("tya");
  asm("pha");

  asm("ldy %v", rgb_ctr);
  asm("lda %v,y", rgb_table);

  asm("sta %w", (unsigned)&ANTIC.wsync);
  asm("sta %w", (unsigned)&GTIA_WRITE.colbk);

  /* FIXME: Still crashing */
//   asm("inc %v", rgb_ctr);
//   asm("lda %v", rgb_ctr);
//   asm("cmp #4");
//   asm("bne %g", __dli_done);

//  asm("lda #0");
//  asm("sta %v", rgb_ctr);

__dli_done:
  asm("pla");
  asm("tay");
  asm("pla");
  asm("rti");
}

void dli_init(void)
{
  rgb_ctr = 0;
  ANTIC.nmien = NMIEN_VBI;
  while (ANTIC.vcount < 124);
  OS.vdslst = (void *) dli;
  ANTIC.nmien = NMIEN_VBI | NMIEN_DLI;
}

void dli_clear(void)
{
  ANTIC.nmien = NMIEN_VBI;
}

#pragma optimize (pop)


void dlist_setup_rgb(unsigned char antic_mode) {
  int l, i;
  unsigned int gfx_ptr1, gfx_ptr2, gfx_ptr3, next_dlist;
  unsigned int scr_mem1, scr_mem2, scr_mem3, dl_idx;
  unsigned char
    gfx_ptr1_hi, gfx_ptr1_lo,
    gfx_ptr2_hi, gfx_ptr2_lo,
    gfx_ptr3_hi, gfx_ptr3_lo;

  scr_mem1 = (unsigned int) scr_mem;
  scr_mem2 = (unsigned int) scr_mem + 8192;
  scr_mem3 = (unsigned int) scr_mem + 16384;

  for (l = 0; l < 3; l++) {
    dl_idx = (l * DLIST_SIZE);

    dlist_mem[dl_idx++] = DL_BLK8;
    dlist_mem[dl_idx++] = DL_BLK8;
    dlist_mem[dl_idx++] = DL_BLK8;

    if (l == 0) {
      gfx_ptr1 = scr_mem1 + 0;
      gfx_ptr2 = scr_mem2 + 40;
      gfx_ptr3 = scr_mem3 + 80;
    } else if (l == 1) {
      gfx_ptr1 = scr_mem2 + 0;
      gfx_ptr2 = scr_mem3 + 40;
      gfx_ptr3 = scr_mem1  + 80;
    } else {
      gfx_ptr1 = scr_mem3 + 0;
      gfx_ptr2 = scr_mem1 + 40;
      gfx_ptr3 = scr_mem2 + 80;
    }

    for (i = 0; i < 64 /* aka 192 / 3 */; i++) {
      /* FIXME: Be more clever */
      gfx_ptr1_hi = (gfx_ptr1 >> 8);
      gfx_ptr1_lo = (gfx_ptr1 & 255);
      gfx_ptr2_hi = (gfx_ptr2 >> 8);
      gfx_ptr2_lo = (gfx_ptr2 & 255);
      gfx_ptr3_hi = (gfx_ptr3 >> 8);
      gfx_ptr3_lo = (gfx_ptr3 & 255);

      dlist_mem[dl_idx++] = DL_LMS(antic_mode);
      dlist_mem[dl_idx++] = gfx_ptr1_lo;
      dlist_mem[dl_idx++] = gfx_ptr1_hi;
      gfx_ptr1 += 120;

      dlist_mem[dl_idx++] = DL_LMS(antic_mode);
      dlist_mem[dl_idx++] = gfx_ptr2_lo;
      dlist_mem[dl_idx++] = gfx_ptr2_hi;
      gfx_ptr2 += 120;

      dlist_mem[dl_idx++] = DL_LMS(antic_mode);
      dlist_mem[dl_idx++] = gfx_ptr3_lo;
      dlist_mem[dl_idx++] = gfx_ptr3_hi;
      gfx_ptr3 += 120;
    }

    if (l == 2) {
      next_dlist = (unsigned int) dlist_mem;
    } else {
      next_dlist = (unsigned int) dlist_mem + (DLIST_SIZE * (l + 1));
    }

    dlist_mem[(l * DLIST_SIZE) + (192 * 3) + 3] = DL_JVB;
    dlist_mem[(l * DLIST_SIZE) + (192 * 3) + 4] = (next_dlist & 255);
    dlist_mem[(l * DLIST_SIZE) + (192 * 3) + 5] = (next_dlist >> 8);
  }

  OS.sdlst = dlist_mem;
}

/* The various graphics modes, the keypresses to
   choose them, and the argument to send to the
   web app via "?mode=". */
enum {
  CHOICE_NONE,
  CHOICE_HIRES_MONO,
  CHOICE_MEDRES_GREY,
  CHOICE_LOWRES_GREY,
  CHOICE_LOWRES_RGB,
  NUM_CHOICES
};

unsigned char choice_keys[NUM_CHOICES] = {
  KEY_NONE,
  KEY_A,
  KEY_B,
  KEY_C,
  KEY_D
};

char * modes[NUM_CHOICES] = {
  "",
  "8",
  "15",
  "9",
  "rgb9"
};


/* The base URL for the web app */
char * default_baseurl = "N:HTTP://billsgames.com/fujinet/apod/index.php";
char * baseurl;

/* Space to store the composed URL */
char url[255];


/* The program! */
void main(void) {
  unsigned char keypress, choice;
  int i, size;

  do {
    /* FIXME Accept command-line options */
//  if (...) {
//    choice = CHOICE_HIRES_MONO;
//    baseurl = default_baseurl;
//  } else {
      /* Prompt user for the preferred viewing mode */
      _graphics(2+16);
      myprint(0, 0, "Astronomy Picture Of");
      myprint(3, 1, "the Day (APOD)");
      myprint(4, 2, "via #FUJINET");
      myprint(1, 3, "bill kendrick 2021");
  
      myprint(0, 5, "A high res mono");
      myprint(0, 6, "B medium res 4 shade");
      myprint(0, 7, "C low res 16 shade");
      myprint(0, 8, "D low res 4096 color");
  
      baseurl = default_baseurl;
  
      /* Accept a choice */
      choice = CHOICE_NONE;
      OS.ch = KEY_NONE;
      do {
        while (OS.ch == KEY_NONE) { OS.color0 = (OS.rtclok[2] >> 2) << 2; }
        keypress = OS.ch;
        OS.ch = KEY_NONE;
  
        for (i = 0; i < NUM_CHOICES; i++) {
          if (choice_keys[i] == keypress) {
            choice = i;
          }
        }
      } while (choice == CHOICE_NONE);
//  }

    /* Set up the display, based on the choice */
    size = 7680;

    OLDVEC = OS.vvblkd;

    if (choice == CHOICE_HIRES_MONO ||
        choice == CHOICE_LOWRES_GREY) {
      dlist_setup(DL_GRAPHICS8);

      if (choice == CHOICE_LOWRES_GREY) {
        /* FIXME: Set up a DLI to keep text window readable */
        OS.gprior = 64;
        OS.color4 = 0; /* Greyscale */
      } else {
        OS.color4 = 128; /* Border (no reason) */
        OS.color2 = 0; /* Background */
        OS.color1 = 15; /* Foreground */
      }
    } else if (choice == CHOICE_MEDRES_GREY) {
      dlist_setup(DL_GRAPHICS15);
  
      OS.color4 = 0; /* Background (black) */
      OS.color1 = 4; /* Dark foreground */
      OS.color2 = 8; /* Medium foreground */
      OS.color3 = 14; /* Light foreground */
    } else if (choice == CHOICE_LOWRES_RGB) {
      size = 7680 * 3;

      dlist_setup_rgb(DL_DLI(DL_GRAPHICS8));
      OS.gprior = 64;
    }

    /* Load the data! */
#ifdef PLACEHOLDER
    for (i = 0; i < size; i++) {
      // scr_mem[i] = POKEY_READ.random;
      scr_mem[i] = i;
    }
#else
    snprintf(url, sizeof(url), "%s?mode=%s", baseurl, modes[choice]);
 
    nopen(1 /* unit 1 */, url, 4 /* read */);
    /* FIXME: Check for error */

    nread(1 /* unit 1 */, scr_mem, (unsigned short) size);
    nclose(1 /* unit 1 */);
#endif

    if (choice == CHOICE_LOWRES_RGB) {
      mySETVBV((void *) VBLANKD);
      dli_init();
    }

    OS.ch = KEY_NONE;
    do { } while(OS.ch == KEY_NONE);
    OS.ch = KEY_NONE;

    dli_clear();  
    mySETVBV((void *) OLDVEC);
  } while(1);
}
