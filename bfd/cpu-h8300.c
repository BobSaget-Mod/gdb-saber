/* BFD library support routines for the Hitachi H8/300 architecture.
   Copyright (C) 1990-1991 Free Software Foundation, Inc.
   Hacked by Steve Chamberlain of Cygnus Support.

This file is part of BFD, the Binary File Descriptor library.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <sysdep.h>
#include "bfd.h"
#include "libbfd.h"

#define DEFINE_TABLE 
#include "h8300-opcode.h"

#define MAXSAME 14

static struct h8_opcode * h8_opcodes_sorted[256][MAXSAME];

/* Run through the opcodes and sort them into order to make them easy
   to disassemble
 */
static void
DEFUN_VOID(bfd_h8_disassemble_init)
{
  unsigned int i;	
  struct h8_opcode *p; 
    
  for (p = h8_opcodes; p->name; p++) {	
    int where = 0;
    int n1 = 0;
    int n2 = 0;
    int n3 = 0;
    int n4= 0;
    if ((int)p->data.nib[0] < 16) {
      n1 =(int) p->data.nib[0] ;
    } else n1 = 0;
    if ((int)p->data.nib[1] < 16) {
      n2 = (int) p->data.nib[1];
    }else n2 = 0;

    for (i = 0; i < MAXSAME; i++) {
      int j = n1 * 16 + n2;
      if (h8_opcodes_sorted[j][i] == (struct h8_opcode *)NULL) {
	h8_opcodes_sorted[j][i] = p;
	break;
      }
    }
    
    if (i==MAXSAME)abort();

    /* Just make sure there are an even number of nibbles in it, and
       that the count is the same s the length */
    for (i = 0; p->data.nib[i] != E; i++) ;
    if (i & 1) abort();
    if (i/2 != p->length) abort();
  }
  for (i = 0; i < 256; i++) 
      {
	if (h8_opcodes_sorted[i][0]) 
	  p = h8_opcodes_sorted[i][0];
	else h8_opcodes_sorted[i][0] = p;
      }
}

unsigned int
DEFUN(bfd_h8_disassemble,(addr, data, stream),
bfd_vma addr AND
CONST bfd_byte *data AND
FILE *stream)
{
  /* Find the first entry in the table for this opcode */
  int rs = 0;
  int rd = 0;
  int rdisp = 0;
  int abs = 0;
  struct h8_opcode **p = h8_opcodes_sorted[(unsigned)(data[0])];
  struct h8_opcode *q;

  /* Find the exact opcode/arg combo */
  while (*p) {
    op_enum_type *nib;
    unsigned int len = 0;
    q = *p++;
    nib  =q->data.nib;
    while (*nib != E) {
      op_enum_type looking_for = *nib;
      int thisnib = data[len>>1] ;
      thisnib = (len & 1) ? (thisnib & 0xf) : ((thisnib >> 4) & 0xf);
      if ((int)looking_for & (int)B31) {
	if (((int)thisnib & 0x8) ==0) goto fail;
	looking_for = (op_enum_type)((int)looking_for &  ~(int)B31);
      }
      if ((int)looking_for & (int)B30) {
	if (((int)thisnib & 0x8) !=0) goto fail;
	looking_for = (op_enum_type)((int)looking_for &  ~(int)B30);
      }
      switch (looking_for) {
      case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7: 
      case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15:
	if ((int)looking_for != thisnib) goto fail;
	break;
      case ABS16SRC:
      case ABS16DST:
      case DISPSRC:
      case DISPDST:
      case IMM16:
	abs = (data[len>>1]) * 256 + data[(len+2)>>1];
	len+=3;
	nib+=3;
	break;
      case DISPREG:
	rdisp = thisnib; 
	break;
      case KBIT:
	abs = thisnib == 0x80 ? 2:1;
	break;
      case IMM8:
      case ABS8SRC:
      case ABS8DST:
      case DISP8:
	abs= data[len>>1];
	len++;
	nib++;
	break;
      case IMM3:
	abs = thisnib ;
	break;
      case RS8:
      case RS16:
      case RSINC:
      case RSIND:
	rs = thisnib;
	break;
      case RD16:
      case RDDEC:
      case RD8:
      case RDIND:
	rd = thisnib;
	break;
      default:
	fprintf(stream, "Dont understand \n");
	goto found;
      }
      len++;
      nib++;
    }
    goto found;
  fail: 
    ;

  }
  fprintf(stream, "%02x %02x        .word\tH'%x,H'%x\n",
	 data[0], data[1],
	 data[0], data[1]);
  return 2;
 found:;
    { int i;

      for (i = 0; i < q->length; i++) {
	fprintf(stream, "%02x ", data[i]);
      }
      for (; i < 6; i++) {
	fprintf(stream, "   ");
      }
    }
  fprintf(stream, "%s\t",q->name);
  /* Now print out the args */
    {
      op_enum_type *args = q->args.nib;
      int hadone = 0;
      while (*args != E) {
	if (hadone)
	  fprintf(stream, ",");
	switch ((int)(*args) & ~((int)B30|(int)B31)) {
	case IMM16:
	case IMM8:
	case IMM3:
	  fprintf(stream, "#H'%x", (unsigned)abs); break;
	case RD8:
	  fprintf(stream, "r%d%c", rd/2, rd & 1 ? 'l' : 'h'); break;
	case RS8:
	  fprintf(stream, "r%d%c", rs/2, rs & 1 ? 'l' : 'h'); break;
	case RD16:
	  fprintf(stream, "r%d", rd& 0x7); break;
	case RS16:
	  fprintf(stream, "r%d", rs & 0x7); break;
	case RSINC:
	  fprintf(stream, "@r%d+", rs & 0x7); break;
	case RDDEC:
	  fprintf(stream, "@-r%d", rd & 0x7); break;
	case RDIND: 
	  fprintf(stream, "@r%d", rd & 0x7); break;
	case RSIND:
	  fprintf(stream, "@r%d",rs & 0x7); break;
	case ABS8SRC:
	case ABS16SRC:
	case ABS16DST:
	case ABS8DST:
	  fprintf(stream, "@H'%x", (unsigned)abs); break;
	case DISP8:
	  fprintf(stream, "%d", (char)abs);break;
	case DISPSRC:
	case DISPDST:
	  fprintf(stream, "@(%d,r%d)", abs, rdisp & 0x7); break;
	case CCR:
	  fprintf(stream, "ccr"); break;
	case KBIT:
	  fprintf(stream, "#%d",abs); break;
	default:
	  abort();
	}
	hadone = 1;
	args++;
      }
    }
  return q->length;
}



unsigned int DEFUN( print_insn_h8300,(addr, data, file),
bfd_vma addr AND
CONST bfd_byte *data AND
PTR file)
{
  static boolean init;
  if (!init) {
    bfd_h8_disassemble_init();
    init= 1;

  }
  return   bfd_h8_disassemble(addr, data, file);
}

/* 
Relocations for the H8

*/
static bfd_reloc_status_enum_type 
DEFUN(howto16_callback,(abfd, reloc_entry, symbol_in, data, ignore_input_section),
bfd *abfd AND
arelent *reloc_entry AND
asymbol *symbol_in AND
unsigned char *data AND
asection *ignore_input_section)
{
  long relocation = 0;
  bfd_vma addr = reloc_entry->address;
  long x = bfd_get_16(abfd, (bfd_byte *)data + addr);

  HOWTO_PREPARE(relocation, symbol_in);

  x = (x + relocation + reloc_entry->addend);

  bfd_put_16(abfd, x, (bfd_byte *)data + addr);
  return bfd_reloc_ok;
}


static bfd_reloc_status_enum_type 
DEFUN(howto8_callback,(abfd, reloc_entry, symbol_in, data, ignore_input_section),
bfd *abfd AND
arelent *reloc_entry AND
asymbol *symbol_in AND
unsigned char *data AND
asection *ignore_input_section)
{
  long relocation = 0;
  bfd_vma addr = reloc_entry->address;
  long x = bfd_get_8(abfd, (bfd_byte *)data + addr);

  HOWTO_PREPARE(relocation, symbol_in);

  x = (x + relocation + reloc_entry->addend);

  bfd_put_8(abfd, x, (bfd_byte *)data + addr);
  return bfd_reloc_ok;
}


static bfd_reloc_status_enum_type 
DEFUN(howto8_FFnn_callback,(abfd, reloc_entry, symbol_in, data, ignore_input_section),
bfd *abfd AND
arelent *reloc_entry AND
asymbol *symbol_in AND
unsigned char *data AND
asection *ignore_input_section)
{
  long relocation = 0;
  bfd_vma addr = reloc_entry->address;

  long x = bfd_get_8(abfd, (bfd_byte *)data + addr);
  abort();
  HOWTO_PREPARE(relocation, symbol_in);

  x = (x + relocation + reloc_entry->addend);

  bfd_put_8(abfd, x, (bfd_byte *)data + addr);
  return bfd_reloc_ok;
}

static bfd_reloc_status_enum_type 
DEFUN(howto8_pcrel_callback,(abfd, reloc_entry, symbol_in, data, ignore_input_section),
bfd *abfd AND
arelent *reloc_entry AND
asymbol *symbol_in AND
unsigned char *data AND
asection *ignore_input_section)
{
  long relocation = 0;
  bfd_vma addr = reloc_entry->address;
  long x = bfd_get_8(abfd, (bfd_byte *)data + addr);
  abort();
  HOWTO_PREPARE(relocation, symbol_in);

  x = (x + relocation + reloc_entry->addend);

  bfd_put_8(abfd, x, (bfd_byte *)data + addr);
  return bfd_reloc_ok;
}


static reloc_howto_type howto_16
  = NEWHOWTO(howto16_callback,"abs16",1,0);
static reloc_howto_type howto_8
  = NEWHOWTO(howto8_callback,"abs8",0,0);

static reloc_howto_type howto_8_FFnn
  = NEWHOWTO(howto8_FFnn_callback,"ff00+abs8",0,0);

static reloc_howto_type howto_8_pcrel
  = NEWHOWTO(howto8_pcrel_callback,"pcrel8",0,1);



static CONST struct reloc_howto_struct  *DEFUN(local_bfd_reloc_type_lookup,( code),
		       bfd_reloc_code_enum_type code)
{
  switch (code) {
  case BFD_RELOC_16:
    return &howto_16;
  case BFD_RELOC_8_FFnn:
    return &howto_8_FFnn;
  case BFD_RELOC_8:
    return &howto_8;
  case BFD_RELOC_8_PCREL:
    return &howto_8_pcrel;
  }
  return (reloc_howto_type *)NULL;
}

int bfd_default_scan_num_mach();

static bfd_arch_info_struct_type arch_info_struct = 
  {
    16,	/* 16 bits in a word */
    16,	/* 16 bits in an address */
    8,	/* 8 bits in a byte */
    bfd_arch_h8300,
    0,	/* only 1 machine */
    "H8/300", /* arch_name */
    "H8/300", /* printable name */
    true,	/* the default machine */
    bfd_default_compatible,
    bfd_default_scan,
    print_insn_h8300,
    local_bfd_reloc_type_lookup,
    0,
  };




void DEFUN_VOID(bfd_h8300_arch)
{
  bfd_arch_linkin(&arch_info_struct);
}



