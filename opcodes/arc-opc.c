/* Opcode table for the ARC.
   Copyright 1994, 1995 Free Software Foundation, Inc.
   Contributed by Doug Evans (dje@cygnus.com).
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include "ansidecl.h"
#include "opcode/arc.h"

#define INSERT_FN(fn) \
static arc_insn fn PARAMS ((arc_insn, const struct arc_operand *, \
			    int, const struct arc_operand_value *, long, \
			    const char **))
#define EXTRACT_FN(fn) \
static long fn PARAMS ((arc_insn *, const struct arc_operand *, \
			int, const struct arc_operand_value **, int *))

INSERT_FN (insert_reg);
INSERT_FN (insert_shimmfinish);
INSERT_FN (insert_limmfinish);
INSERT_FN (insert_shimmoffset);
INSERT_FN (insert_shimmzero);
INSERT_FN (insert_flag);
INSERT_FN (insert_flagfinish);
INSERT_FN (insert_cond);
INSERT_FN (insert_forcelimm);
INSERT_FN (insert_reladdr);
INSERT_FN (insert_absaddr);
INSERT_FN (insert_unopmacro);
INSERT_FN (insert_multshift);

EXTRACT_FN (extract_reg);
EXTRACT_FN (extract_flag);
EXTRACT_FN (extract_cond);
EXTRACT_FN (extract_reladdr);
EXTRACT_FN (extract_unopmacro);
EXTRACT_FN (extract_multshift);

/* Various types of ARC operands, including insn suffixes.  */

/* Insn format values:

   'a'	REGA		register A field
   'b'	REGB		register B field
   'c'	REGC		register C field
   'S'	SHIMMFINISH	finish inserting a shimm value
   'L'	LIMMFINISH	finish inserting a limm value
   'd'	SHIMMOFFSET	shimm offset in ld,st insns
   '0'	SHIMMZERO	0 shimm value in ld,st insns
   'f'	FLAG		F flag
   'F'	FLAGFINISH	finish inserting the F flag
   'G'	FLAGINSN	insert F flag in "flag" insn
   'n'	DELAY		N field (nullify field)
   'q'	COND		condition code field
   'Q'	FORCELIMM	set `cond_p' to 1 to ensure a constant is a limm
   'B'	BRANCH		branch address (22 bit pc relative)
   'J'	JUMP		jump address (26 bit absolute)
   'z'	SIZE1		size field in ld a,[b,c]
   'Z'	SIZE10		size field in ld a,[b,shimm]
   'y'	SIZE22		size field in st c,[b,shimm]
   'x'	SIGN0		sign extend field ld a,[b,c]
   'X'	SIGN9		sign extend field ld a,[b,shimm]
   'w'	ADDRESS3	write-back field in ld a,[b,c]
   'W'	ADDRESS12	write-back field in ld a,[b,shimm]
   'v'	ADDRESS24	write-back field in st c,[b,shimm]
   'e'	CACHEBYPASS5	cache bypass in ld a,[b,c]
   'E'	CACHEBYPASS14	cache bypass in ld a,[b,shimm]
   'D'	CACHEBYPASS26	cache bypass in st c,[b,shimm]
   'u'	UNSIGNED	unsigned multiply
   's'	SATURATION	saturation limit in audio arc mac insn
   'U'	UNOPMACRO	fake operand to copy REGB to REGC for unop macros

   The following modifiers may appear between the % and char (eg: %.f):

   '.'	MODDOT		'.' prefix must be present
   'r'	REG		generic register value, for register table
   'A'	AUXREG		auxiliary register in lr a,[b], sr c,[b]

   Fields are:

   CHAR BITS SHIFT FLAGS INSERT_FN EXTRACT_FN
*/

const struct arc_operand arc_operands[] =
{
/* place holder (??? not sure if needed) */
#define UNUSED 0
  { 0 },

/* register A or shimm/limm indicator */
#define REGA (UNUSED + 1)
  { 'a', 6, ARC_SHIFT_REGA, 0, insert_reg, extract_reg },

/* register B or shimm/limm indicator */
#define REGB (REGA + 1)
  { 'b', 6, ARC_SHIFT_REGB, 0, insert_reg, extract_reg },

/* register C or shimm/limm indicator */
#define REGC (REGB + 1)
  { 'c', 6, ARC_SHIFT_REGC, 0, insert_reg, extract_reg },

/* fake operand used to insert shimm value into most instructions */
#define SHIMMFINISH (REGC + 1)
  { 'S', 9, 0, ARC_OPERAND_SIGNED + ARC_OPERAND_FAKE, insert_shimmfinish, 0 },

/* fake operand used to insert limm value into most instructions;
   this is also used for .word handling  */
#define LIMMFINISH (SHIMMFINISH + 1)
  { 'L', 32, 32, ARC_OPERAND_ADDRESS + ARC_OPERAND_LIMM + ARC_OPERAND_FAKE, insert_limmfinish, 0 },

/* shimm operand when there is no reg indicator (ld,st) */
#define SHIMMOFFSET (LIMMFINISH + 1)
  { 'd', 9, 0, ARC_OPERAND_SIGNED, insert_shimmoffset, 0 },

/* 0 shimm operand for ld,st insns */
#define SHIMMZERO (SHIMMOFFSET + 1)
  { '0', 9, 0, ARC_OPERAND_FAKE, insert_shimmzero, 0 },

/* flag update bit (insertion is defered until we know how) */
#define FLAG (SHIMMZERO + 1)
  { 'f', 1, 8, ARC_OPERAND_SUFFIX, insert_flag, extract_flag },

/* fake utility operand to finish 'f' suffix handling */
#define FLAGFINISH (FLAG + 1)
  { 'F', 1, 8, ARC_OPERAND_FAKE, insert_flagfinish, 0 },

/* fake utility operand to set the 'f' flag for the "flag" insn */
#define FLAGINSN (FLAGFINISH + 1)
  { 'G', 1, 8, ARC_OPERAND_FAKE, insert_flag, 0 },

/* branch delay types */
#define DELAY (FLAGINSN + 1)
  { 'n', 2, 5, ARC_OPERAND_SUFFIX },

/* conditions */
#define COND (DELAY + 1)
  { 'q', 5, 0, ARC_OPERAND_SUFFIX, insert_cond, extract_cond },

/* set `cond_p' to 1 to ensure a constant is treated as a limm */
#define FORCELIMM (COND + 1)
  { 'Q', 0, 0, ARC_OPERAND_FAKE, insert_forcelimm },

/* branch address; b, bl, and lp insns */
#define BRANCH (FORCELIMM + 1)
  { 'B', 20, 7, ARC_OPERAND_RELATIVE_BRANCH + ARC_OPERAND_SIGNED, insert_reladdr, extract_reladdr },

/* jump address; j insn (this is basically the same as 'L' except that the
   value is right shifted by 2); this is also used for .word handling */
#define JUMP (BRANCH + 1)
  { 'J', 24, 32, ARC_OPERAND_ABSOLUTE_BRANCH + ARC_OPERAND_LIMM + ARC_OPERAND_FAKE, insert_absaddr },

/* size field, stored in bit 1,2 */
#define SIZE1 (JUMP + 1)
  { 'z', 2, 1, ARC_OPERAND_SUFFIX },

/* size field, stored in bit 10,11 */
#define SIZE10 (SIZE1 + 1)
  { 'Z', 2, 10, ARC_OPERAND_SUFFIX, },

/* size field, stored in bit 22,23 */
#define SIZE22 (SIZE10 + 1)
  { 'y', 2, 22, ARC_OPERAND_SUFFIX, },

/* sign extend field, stored in bit 0 */
#define SIGN0 (SIZE22 + 1)
  { 'x', 1, 0, ARC_OPERAND_SUFFIX },

/* sign extend field, stored in bit 9 */
#define SIGN9 (SIGN0 + 1)
  { 'X', 1, 9, ARC_OPERAND_SUFFIX },

/* address write back, stored in bit 3 */
#define ADDRESS3 (SIGN9 + 1)
  { 'w', 1, 3, ARC_OPERAND_SUFFIX },

/* address write back, stored in bit 12 */
#define ADDRESS12 (ADDRESS3 + 1)
  { 'W', 1, 12, ARC_OPERAND_SUFFIX },

/* address write back, stored in bit 24 */
#define ADDRESS24 (ADDRESS12 + 1)
  { 'v', 1, 24, ARC_OPERAND_SUFFIX },

/* cache bypass, stored in bit 5 */
#define CACHEBYPASS5 (ADDRESS24 + 1)
  { 'e', 1, 5, ARC_OPERAND_SUFFIX },

/* cache bypass, stored in bit 14 */
#define CACHEBYPASS14 (CACHEBYPASS5 + 1)
  { 'E', 1, 14, ARC_OPERAND_SUFFIX },

/* cache bypass, stored in bit 26 */
#define CACHEBYPASS26 (CACHEBYPASS14 + 1)
  { 'D', 1, 26, ARC_OPERAND_SUFFIX },

/* unsigned multiply */
#define UNSIGNED (CACHEBYPASS26 + 1)
  { 'u', 1, 27, ARC_OPERAND_SUFFIX },

/* unsigned multiply */
#define SATURATION (UNSIGNED + 1)
  { 's', 1, 28, ARC_OPERAND_SUFFIX },

/* unop macro, used to copy REGB to REGC */
#define UNOPMACRO (SATURATION + 1)
  { 'U', 6, ARC_SHIFT_REGC, ARC_OPERAND_FAKE, insert_unopmacro, extract_unopmacro },

/* '.' modifier ('.' required).  */
#define MODDOT (UNOPMACRO + 1)
  { '.', 1, 0, ARC_MOD_DOT },

/* Dummy 'r' modifier for the register table.
   It's called a "dummy" because there's no point in inserting an 'r' into all
   the %a/%b/%c occurrences in the insn table.  */
#define REG (MODDOT + 1)
  { 'r', 6, 0, ARC_MOD_REG },

/* Known auxiliary register modifier (stored in shimm field).  */
#define AUXREG (REG + 1)
  { 'A', 9, 0, ARC_MOD_AUXREG },

/* end of list place holder */
  { 0 }
};

/* Given a format letter, yields the index into `arc_operands'.
   eg: arc_operand_map['a'] = REGA.  */
unsigned char arc_operand_map[256];

#define I(x) (((x) & 31) << 27)
#define A(x) (((x) & ARC_MASK_REG) << ARC_SHIFT_REGA)
#define B(x) (((x) & ARC_MASK_REG) << ARC_SHIFT_REGB)
#define C(x) (((x) & ARC_MASK_REG) << ARC_SHIFT_REGC)
#define R(x,b,m) (((x) & (m)) << (b))	/* value X, mask M, at bit B */

/* ARC instructions (sorted by at least the first letter, and equivalent
   opcodes kept together).

   By recording the insns this way, the table is not hashable on the opcode.
   That's not a real loss though as there are only a few entries for each
   insn (ld/st being the exception), which are quickly found and since
   they're stored together (eg: all `ld' variants are together) very little
   time is spent on the opcode itself.  The slow part is parsing the options,
   but that's always going to be slow.

   Longer versions of insns must appear before shorter ones (if gas sees
   "lsr r2,r3,1" when it's parsing "lsr %a,%b" it will think the ",1" is
   junk).

   This table is best viewed on a wide screen (161 columns).
   I'd prefer to keep it this way.  */

/* ??? This table also includes macros: asl, lsl, and mov.  The ppc port has
   a more general facility for dealing with macros which could be used if
   we need to.  */
/* ??? As an experiment, the "mov" macro appears at the start so it is
   prefered to "and" when disassembling.  At present, the table needn't be
   sorted, though all opcodes with the same first letter must be kept
   together.  */

const struct arc_opcode arc_opcodes[] = {
  { "mac%u%.s%.q%.f %a,%b,%c%F%S%L",	I(-4),		I(24),		ARC_MACH_AUDIO },
  /* Note that "mov" is really an "and".  */
  { "mov%.q%.f %a,%b%F%S%L%U",		I(-1),		I(12) },
  { "mul%u%.q%.f %a,%b,%c%F%S%L",	I(-2),		I(28),		ARC_MACH_AUDIO },
  /* ??? This insn allows an optional "0," preceding the args.  */
  /* We can't use %u here because it's not a suffix (the "64" is in the way).  */
  { "mul64%.q%.f %b,%c%F%S%L",		I(-1)+A(-1),	I(20)+A(-1),	ARC_MACH_HOST+ARC_MACH_GRAPHICS },
  { "mulu64%.q%.f %b,%c%F%S%L",		I(-1)+A(-1),	I(21)+A(-1),	ARC_MACH_HOST+ARC_MACH_GRAPHICS },

  { "adc%.q%.f %a,%b,%c%F%S%L",		I(-1),		I(9) },
  { "add%.q%.f %a,%b,%c%F%S%L",		I(-1),		I(8) },
  { "and%.q%.f %a,%b,%c%F%S%L",		I(-1),		I(12) },
  { "asl%.q%.f %a,%b,%c%F%S%L",		I(-1),		I(16),		ARC_MACH_HOST+ARC_MACH_GRAPHICS },
  /* Note that "asl" is really an "add".  */
  { "asl%.q%.f %a,%b%F%S%L%U",		I(-1),		I(8) },
  { "asr%.q%.f %a,%b,%c%F%S%L",		I(-1),		I(18),		ARC_MACH_HOST+ARC_MACH_GRAPHICS },
  { "asr%.q%.f %a,%b%F%S%L",		I(-1)+C(-1),	I(3)+C(1) },
  { "bic%.q%.f %a,%b,%c%F%S%L",		I(-1),		I(14) },
  { "b%q%.n %B",			I(-1),		I(4) },
  { "bl%q%.n %B",			I(-1),		I(5) },
  { "extb%.q%.f %a,%b%F%S%L",		I(-1)+C(-1),	I(3)+C(7) },
  { "extw%.q%.f %a,%b%F%S%L",		I(-1)+C(-1),	I(3)+C(8) },
  { "flag%.q %b%G%S%L",			I(-1)+A(-1)+C(-1),		I(3)+A(ARC_REG_SHIMM_UPDATE)+C(0) },
  /* %Q: force cond_p=1 --> no shimm values */
  /* ??? This insn allows an optional flags spec.  */
  { "j%q%Q%.n%.f %b%J",			I(-1)+A(-1)+C(-1)+R(-1,7,1),	I(7)+A(0)+C(0)+R(0,7,1) },
  /* Put opcode 1 ld insns first so shimm gets prefered over limm.  */
  /* "[%b]" is before "[%b,%d]" so 0 offsets don't get printed.  */
  { "ld%Z%.X%.W%.E %0%a,[%b]%L",	I(-1)+R(-1,13,1)+R(-1,0,511),	I(1)+R(0,13,1)+R(0,0,511) },
  { "ld%Z%.X%.W%.E %a,[%b,%d]%S%L",	I(-1)+R(-1,13,1),		I(1)+R(0,13,1) },
  { "ld%z%.x%.w%.e %a,[%b,%c]",		I(-1)+R(-1,4,1)+R(-1,6,7),	I(0)+R(0,4,1)+R(0,6,7) },
  { "lp%q%.n %B",			I(-1),		I(6), },
  { "lr %a,[%Ab]%S%L",			I(-1)+C(-1),	I(1)+C(0x10) },
  /* Note that "lsl" is really an "add".  */
  { "lsl%.q%.f %a,%b%F%S%L%U",		I(-1),		I(8) },
  { "lsr%.q%.f %a,%b,%c%F%S%L",		I(-1),		I(17),		ARC_MACH_HOST+ARC_MACH_GRAPHICS },
  { "lsr%.q%.f %a,%b%F%S%L",		I(-1)+C(-1),	I(3)+C(2) },
  /* Note that "nop" is really an "xor".  */
  { "nop",				0xffffffff,	0x7fffffff },
  { "or%.q%.f %a,%b,%c%F%S%L",		I(-1),		I(13) },
  /* ??? The %a here should be %p or something.  */
  { "padc%.q%.f %a,%b,%c%F%S%L",	I(-1),		I(25),		ARC_MACH_GRAPHICS },
  { "padd%.q%.f %a,%b,%c%F%S%L",	I(-1),		I(24),		ARC_MACH_GRAPHICS },
  /* Note that "pmov" is really a "pand".  */
  { "pmov%.q%.f %a,%b%F%S%L%U",		I(-1),		I(28),		ARC_MACH_GRAPHICS },
  { "pand%.q%.f %a,%b,%c%F%S%L",	I(-1),		I(28),		ARC_MACH_GRAPHICS },
  { "psbc%.q%.f %a,%b,%c%F%S%L",	I(-1),		I(27),		ARC_MACH_GRAPHICS },
  { "psub%.q%.f %a,%b,%c%F%S%L",	I(-1),		I(26),		ARC_MACH_GRAPHICS },
  /* Note that "rlc" is really an "adc".  */
  { "rlc%.q%.f %a,%b%F%S%L%U",		I(-1),		I(9) },
  { "ror%.q%.f %a,%b,%c%F%S%L",		I(-1),		I(19),		ARC_MACH_HOST+ARC_MACH_GRAPHICS },
  { "ror%.q%.f %a,%b%F%S%L",		I(-1)+C(-1),	I(3)+C(3) },
  { "rrc%.q%.f %a,%b%F%S%L",		I(-1)+C(-1),	I(3)+C(4) },
  { "sbc%.q%.f %a,%b,%c%F%S%L",		I(-1),		I(11) },
  { "sexb%.q%.f %a,%b%F%S%L",		I(-1)+C(-1),	I(3)+C(5) },
  { "sexw%.q%.f %a,%b%F%S%L",		I(-1)+C(-1),	I(3)+C(6) },
  { "sr %c,[%Ab]%S%L",			I(-1)+A(-1),		I(2)+A(0x10) },
  /* "[%b]" is before "[%b,%d]" so 0 offsets don't get printed.  */
  { "st%y%.v%.D %0%c,[%b]%L",		I(-1)+R(-1,25,3)+R(-1,21,1)+R(-1,0,511),	I(2)+R(0,25,3)+R(0,21,1)+R(0,0,511) },
  { "st%y%.v%.D %c,[%b,%d]%S%L",	I(-1)+R(-1,25,3)+R(-1,21,1),			I(2)+R(0,25,3)+R(0,21,1) },
  { "sub%.q%.f %a,%b,%c%F%S%L",		I(-1),		I(10) },
  { "swap%.q%.f %a,%b%F%S%L",		I(-1)+C(-1),	I(3)+C(9),	ARC_MACH_AUDIO },
  { "xor%.q%.f %a,%b,%c%F%S%L",		I(-1),		I(15) }
};
const int arc_opcodes_count = sizeof (arc_opcodes) / sizeof (arc_opcodes[0]);

const struct arc_operand_value arc_reg_names[] =
{
  /* Sort this so that the first 61 entries are sequential.
     IE: For each i (i<61), arc_reg_names[i].value == i.  */

  { "r0", 0, REG }, { "r1", 1, REG }, { "r2", 2, REG }, { "r3", 3, REG },
  { "r4", 4, REG }, { "r5", 5, REG }, { "r6", 6, REG }, { "r7", 7, REG },
  { "r8", 8, REG }, { "r9", 9, REG }, { "r10", 10, REG }, { "r11", 11, REG },
  { "r12", 12, REG }, { "r13", 13, REG }, { "r14", 14, REG }, { "r15", 15, REG },
  { "r16", 16, REG }, { "r17", 17, REG }, { "r18", 18, REG }, { "r19", 19, REG },
  { "r20", 20, REG }, { "r21", 21, REG }, { "r22", 22, REG }, { "r23", 23, REG },
  { "r24", 24, REG }, { "r25", 25, REG }, { "r26", 26, REG }, { "fp", 27, REG },
  { "sp", 28, REG }, { "ilink1", 29, REG }, { "ilink2", 30, REG }, { "blink", 31, REG },
  { "r32", 32, REG }, { "r33", 33, REG }, { "r34", 34, REG }, { "r35", 35, REG },
  { "r36", 36, REG }, { "r37", 37, REG }, { "r38", 38, REG }, { "r39", 39, REG },
  { "r40", 40, REG }, { "r41", 41, REG }, { "r42", 42, REG }, { "r43", 43, REG },
  { "r44", 44, REG }, { "r45", 45, REG }, { "r46", 46, REG }, { "r47", 47, REG },
  { "r48", 48, REG }, { "r49", 49, REG }, { "r50", 50, REG }, { "r51", 51, REG },
  { "r52", 52, REG }, { "r53", 53, REG }, { "r54", 54, REG }, { "r55", 55, REG },
  { "r56", 56, REG }, { "r57", 57, REG }, { "r58", 58, REG }, { "r59", 59, REG },
  { "lp_count", 60, REG },

  /* I'd prefer to output these as "fp" and "sp" by default, but we still need
     to recognize the canonical values.  */
  { "r27", 27, REG }, { "r28", 28, REG },

  /* Standard auxiliary registers.  */
  { "status",	0, AUXREG },
  { "semaphore", 1, AUXREG },
  { "lp_start",	2, AUXREG },
  { "lp_end",	3, AUXREG },
  { "identity",	4, AUXREG },
  { "debug",	5, AUXREG },

  /* Host ARC Extensions.  */
  { "mlo",	57, REG, ARC_MACH_HOST },
  { "mmid",	58, REG, ARC_MACH_HOST },
  { "mhi",	59, REG, ARC_MACH_HOST },
  { "ivic",	0x10, AUXREG, ARC_MACH_HOST },
  { "ivdc",	0x11, AUXREG, ARC_MACH_HOST },
  { "ivdcn",	0x12, AUXREG, ARC_MACH_HOST },
  { "flushd",	0x13, AUXREG, ARC_MACH_HOST },
  { "saha",	0x14, AUXREG, ARC_MACH_HOST },
  { "gahd",	0x15, AUXREG, ARC_MACH_HOST },
  { "aahd",	0x16, AUXREG, ARC_MACH_HOST },
  { "rrcr",	0x17, AUXREG, ARC_MACH_HOST },
  { "rpcr",	0x18, AUXREG, ARC_MACH_HOST },
  { "flushdn",	0x19, AUXREG, ARC_MACH_HOST },
  { "dbgad1",	0x1a, AUXREG, ARC_MACH_HOST },
  { "dbgad2",	0x1b, AUXREG, ARC_MACH_HOST },
  { "dbgmde",	0x1c, AUXREG, ARC_MACH_HOST },
  { "dbgstat",	0x1d, AUXREG, ARC_MACH_HOST },
  { "wag",	0x1e, AUXREG, ARC_MACH_HOST },
  { "mulhi",	0x1f, AUXREG, ARC_MACH_HOST },
  { "intwide",	0x20, AUXREG, ARC_MACH_HOST },
  { "intgen",	0x21, AUXREG, ARC_MACH_HOST },
  { "rfsh_n",	0x22, AUXREG, ARC_MACH_HOST },

  /* Graphics ARC Extensions.  */
  { "mlo",	57, REG, ARC_MACH_GRAPHICS },
  { "mmid",	58, REG, ARC_MACH_GRAPHICS },
  { "mhi",	59, REG, ARC_MACH_GRAPHICS },
  { "ivic",	0x10, AUXREG, ARC_MACH_GRAPHICS },
  { "wag",	0x1e, AUXREG, ARC_MACH_GRAPHICS },
  { "mulhi",	0x1f, AUXREG, ARC_MACH_GRAPHICS },
  { "intwide",	0x20, AUXREG, ARC_MACH_GRAPHICS },
  { "intgen",	0x21, AUXREG, ARC_MACH_GRAPHICS },
  { "pix",	0x100, AUXREG, ARC_MACH_GRAPHICS },
  { "scratch",	0x120, AUXREG, ARC_MACH_GRAPHICS },

  /* Audio ARC Extensions.  */
  { "macmode",	39, REG, ARC_MACH_AUDIO },
  { "rs1",	40, REG, ARC_MACH_AUDIO },
  { "rs1n",	41, REG, ARC_MACH_AUDIO },
  { "rs1start",	42, REG, ARC_MACH_AUDIO },
  { "rs1size",	43, REG, ARC_MACH_AUDIO },
  { "rs1delta",	44, REG, ARC_MACH_AUDIO },
  { "rs1pos",	45, REG, ARC_MACH_AUDIO },
  { "rd1",	46, REG, ARC_MACH_AUDIO },
  { "rd1n",	47, REG, ARC_MACH_AUDIO },
  { "rd1d",	48, REG, ARC_MACH_AUDIO },
  { "rd1pos",	49, REG, ARC_MACH_AUDIO },
  { "rs2",	50, REG, ARC_MACH_AUDIO },
  { "rs2n",	51, REG, ARC_MACH_AUDIO },
  { "rs2start",	52, REG, ARC_MACH_AUDIO },
  { "rs2size",	53, REG, ARC_MACH_AUDIO },
  { "rs2delta",	54, REG, ARC_MACH_AUDIO },
  { "rs2pos",	55, REG, ARC_MACH_AUDIO },
  { "rd2",	56, REG, ARC_MACH_AUDIO },
  { "rd2n",	57, REG, ARC_MACH_AUDIO },
  { "rd2d",	58, REG, ARC_MACH_AUDIO },
  { "rd2pos",	59, REG, ARC_MACH_AUDIO },
  { "ivic",	0x10, AUXREG, ARC_MACH_AUDIO },
  { "wag",	0x1e, AUXREG, ARC_MACH_AUDIO },
  { "intwide",	0x20, AUXREG, ARC_MACH_AUDIO },
  { "intgen",	0x21, AUXREG, ARC_MACH_AUDIO },
  { "bm_sstart", 0x30, AUXREG, ARC_MACH_AUDIO },
  { "bm_length", 0x31, AUXREG, ARC_MACH_AUDIO },
  { "bm_rstart", 0x32, AUXREG, ARC_MACH_AUDIO },
  { "bm_go",	0x33, AUXREG, ARC_MACH_AUDIO },
  { "xtp_newval", 0x40, AUXREG, ARC_MACH_AUDIO },
  { "sram",	0x400, AUXREG, ARC_MACH_AUDIO },
  { "reg_file",	0x800, AUXREG, ARC_MACH_AUDIO },
};
const int arc_reg_names_count = sizeof (arc_reg_names) / sizeof (arc_reg_names[0]);

/* The suffix table.
   Operands with the same name must be stored together.  */

const struct arc_operand_value arc_suffixes[] =
{
  /* Entry 0 is special, default values aren't printed by the disassembler.  */
  { "", 0, -1 },
  { "al", 0, COND },
  { "ra", 0, COND },
  { "eq", 1, COND },
  { "z", 1, COND },
  { "ne", 2, COND },
  { "nz", 2, COND },
  { "p", 3, COND },
  { "pl", 3, COND },
  { "n", 4, COND },
  { "mi", 4, COND },
  { "c", 5, COND },
  { "cs", 5, COND },
  { "lo", 5, COND },
  { "nc", 6, COND },
  { "cc", 6, COND },
  { "hs", 6, COND },
  { "v", 7, COND },
  { "vs", 7, COND },
  { "nv", 8, COND },
  { "vc", 8, COND },
  { "gt", 9, COND },
  { "ge", 10, COND },
  { "lt", 11, COND },
  { "le", 12, COND },
  { "hi", 13, COND },
  { "ls", 14, COND },
  { "pnz", 15, COND },
  { "f", 1, FLAG },
  { "nd", 0, DELAY },
  { "d", 1, DELAY },
  { "jd", 2, DELAY },
/*{ "b", 7, SIZEEXT },*/
/*{ "b", 5, SIZESEX },*/
  { "b", 1, SIZE1 },
  { "b", 1, SIZE10 },
  { "b", 1, SIZE22 },
/*{ "w", 8, SIZEEXT },*/
/*{ "w", 6, SIZESEX },*/
  { "w", 2, SIZE1 },
  { "w", 2, SIZE10 },
  { "w", 2, SIZE22 },
  { "x", 1, SIGN0 },
  { "x", 1, SIGN9 },
  { "a", 1, ADDRESS3 },
  { "a", 1, ADDRESS12 },
  { "a", 1, ADDRESS24 },
  { "di", 1, CACHEBYPASS5 },
  { "di", 1, CACHEBYPASS14 },
  { "di", 1, CACHEBYPASS26 },

  /* Audio ARC Extensions.  */
  /* ??? The values here are guesses.  */
  { "ss", 16, COND, ARC_MACH_AUDIO },
  { "sc", 17, COND, ARC_MACH_AUDIO },
  { "mh", 18, COND, ARC_MACH_AUDIO },
  { "ml", 19, COND, ARC_MACH_AUDIO },
};
const int arc_suffixes_count = sizeof (arc_suffixes) / sizeof (arc_suffixes[0]);

/* Configuration flags.  */

/* Various ARC_HAVE_XXX bits.  */
static int cpu_type;

/* Translate a bfd_mach_arc_xxx value to a ARC_MACH_XXX value.  */

int
arc_get_opcode_mach (bfd_mach, big_p)
     int bfd_mach, big_p;
{
  static int mach_type_map[] =
    {
      ARC_MACH_BASE, ARC_MACH_HOST, ARC_MACH_GRAPHICS, ARC_MACH_AUDIO
    };

  return mach_type_map[bfd_mach] | (big_p ? ARC_MACH_BIG : 0);
}

/* Initialize any tables that need it.
   Must be called once at start up (or when first needed).

   FLAGS is a set of bits that say what version of the cpu we have,
   and in particular at least (one of) ARC_MACH_XXX.  */

void
arc_opcode_init_tables (flags)
     int flags;
{
  register int i,n;
  static int map_init_p = 0;

  cpu_type = flags;

  /* We may be intentionally called more than once (for example gdb will call
     us each time the user switches cpu).  This table only needs to be init'd
     once though.  */
  if (!map_init_p)
    {
      memset (arc_operand_map, 0, sizeof (arc_operand_map));
      n = sizeof (arc_operands) / sizeof (arc_operands[0]);
      for (i = 0; i < n; i++)
	arc_operand_map[arc_operands[i].fmt] = i;
      map_init_p = 1;
    }
}

/* Return non-zero if OPCODE is supported on the specified cpu.
   Cpu selection is made when calling `arc_opcode_init_tables'.  */

int
arc_opcode_supported (opcode)
     const struct arc_opcode *opcode;
{
  if (ARC_OPCODE_CPU (opcode->flags) == 0)
    return 1;
  if (ARC_OPCODE_CPU (opcode->flags) & ARC_HAVE_CPU (cpu_type))
    return 1;
  return 0;
}

/* Return non-zero if OPVAL is supported on the specified cpu.
   Cpu selection is made when calling `arc_opcode_init_tables'.  */

int
arc_opval_supported (opval)
     const struct arc_operand_value *opval;
{
  if (ARC_OPVAL_CPU (opval->flags) == 0)
    return 1;
  if (ARC_OPVAL_CPU (opval->flags) & ARC_HAVE_CPU (cpu_type))
    return 1;
  return 0;
}

/* Nonzero if we've seen an 'f' suffix (in certain insns).  */
static int flag_p;

/* Nonzero if we've finished processing the 'f' suffix.  */
static int flagshimm_handled_p;

/* Nonzero if we've seen a 'q' suffix (condition code).  */
static int cond_p;

/* Nonzero if we've inserted a shimm.  */
static int shimm_p;

/* The value of the shimm we inserted (each insn only gets one but it can
   appear multiple times.  */
static int shimm;

/* Nonzero if we've inserted a limm (during assembly) or seen a limm
   (during disassembly).  */
static int limm_p;

/* The value of the limm we inserted.  Each insn only gets one but it can
   appear multiple times.  */
static long limm;

/* Insertion functions.  */

/* Called by the assembler before parsing an instruction.  */

void
arc_opcode_init_insert ()
{
  flag_p = 0;
  flagshimm_handled_p = 0;
  cond_p = 0;
  shimm_p = 0;
  limm_p = 0;
}

/* Called by the assembler to see if the insn has a limm operand.
   Also called by the disassembler to see if the insn contains a limm.  */

int
arc_opcode_limm_p (limmp)
     long *limmp;
{
  if (limmp)
    *limmp = limm;
  return limm_p;
}

/* Insert a value into a register field.
   If REG is NULL, then this is actually a constant.

   We must also handle auxiliary registers for lr/sr insns.  */

static arc_insn
insert_reg (insn, operand, mods, reg, value, errmsg)
     arc_insn insn;
     const struct arc_operand *operand;
     int mods;
     const struct arc_operand_value *reg;
     long value;
     const char **errmsg;
{
  static char buf[100];

  if (!reg)
    {
      /* We have a constant that also requires a value stored in a register
	 field.  Handle these by updating the register field and saving the
	 value for later handling by either %S (shimm) or %L (limm).  */

      /* Try to use a shimm value before a limm one.  */
      if (ARC_SHIMM_CONST_P (value)
	  /* If we've seen a conditional suffix we have to use a limm.  */
	  && !cond_p
	  /* If we already have a shimm value that is different than ours
	     we have to use a limm.  */
	  && (!shimm_p || shimm == value))
	{
	  int marker = flag_p ? ARC_REG_SHIMM_UPDATE : ARC_REG_SHIMM;
	  flagshimm_handled_p = 1;
	  shimm_p = 1;
	  shimm = value;
	  insn |= marker << operand->shift;
	  /* insn |= value & 511; - done later */
	}
      /* We have to use a limm.  If we've already seen one they must match.  */
      else if (!limm_p || limm == value)
	{
	  limm_p = 1;
	  limm = value;
	  insn |= ARC_REG_LIMM << operand->shift;
	  /* The constant is stored later.  */
	}
      else
	{
	  *errmsg = "unable to fit different valued constants into instruction";
	}
    }
  else
    {
      /* We have to handle both normal and auxiliary registers.  */

      if (reg->type == AUXREG)
	{
	  if (!(mods & ARC_MOD_AUXREG))
	    *errmsg = "auxiliary register not allowed here";
	  else
	    {
	      insn |= ARC_REG_SHIMM << operand->shift;
	      insn |= reg->value << arc_operands[reg->type].shift;
	    }
	}
      else
	{
	  /* We should never get an invalid register number here.  */
	  if ((unsigned int) reg->value > 60)
	    {
	      sprintf (buf, "invalid register number `%d'", reg->value);
	      *errmsg = buf;
	    }
	  else
	    insn |= reg->value << operand->shift;
	}
    }

  return insn;
}

/* Called when we see an 'f' flag.  */

static arc_insn
insert_flag (insn, operand, mods, reg, value, errmsg)
     arc_insn insn;
     const struct arc_operand *operand;
     int mods;
     const struct arc_operand_value *reg;
     long value;
     const char **errmsg;
{
  /* We can't store anything in the insn until we've parsed the registers.
     Just record the fact that we've got this flag.  `insert_reg' will use it
     to store the correct value (ARC_REG_SHIMM_UPDATE or bit 0x100).  */
  flag_p = 1;

  return insn;
}

/* Called after completely building an insn to ensure the 'f' flag gets set
   properly.  This is needed because we don't know how to set this flag until
   we've parsed the registers.  */

static arc_insn
insert_flagfinish (insn, operand, mods, reg, value, errmsg)
     arc_insn insn;
     const struct arc_operand *operand;
     int mods;
     const struct arc_operand_value *reg;
     long value;
     const char **errmsg;
{
  if (flag_p && !flagshimm_handled_p)
    {
      if (shimm_p)
	abort ();
      flagshimm_handled_p = 1;
      insn |= (1 << operand->shift);
    }
  return insn;
}

/* Called when we see a conditional flag (eg: .eq).  */

static arc_insn
insert_cond (insn, operand, mods, reg, value, errmsg)
     arc_insn insn;
     const struct arc_operand *operand;
     int mods;
     const struct arc_operand_value *reg;
     long value;
     const char **errmsg;
{
  cond_p = 1;
  insn |= (value & ((1 << operand->bits) - 1)) << operand->shift;
  return insn;
}

/* Used in the "j" instruction to prevent constants from being interpreted as
   shimm values (which the jump insn doesn't accept).  This can also be used
   to force the use of limm values in other situations (eg: ld r0,[foo] uses
   this).
   ??? The mechanism is sound.  Access to it is a bit klunky right now.  */

static arc_insn
insert_forcelimm (insn, operand, mods, reg, value, errmsg)
     arc_insn insn;
     const struct arc_operand *operand;
     int mods;
     const struct arc_operand_value *reg;
     long value;
     const char **errmsg;
{
  cond_p = 1;
  return insn;
}

/* Used in ld/st insns to handle the shimm offset field.  */

static arc_insn
insert_shimmoffset (insn, operand, mods, reg, value, errmsg)
     arc_insn insn;
     const struct arc_operand *operand;
     int mods;
     const struct arc_operand_value *reg;
     long value;
     const char **errmsg;
{
  insn |= (value & ((1 << operand->bits) - 1)) << operand->shift;
  return insn;
}

/* Used in ld/st insns when the shimm offset is 0.  */

static arc_insn
insert_shimmzero (insn, operand, mods, reg, value, errmsg)
     arc_insn insn;
     const struct arc_operand *operand;
     int mods;
     const struct arc_operand_value *reg;
     long value;
     const char **errmsg;
{
  shimm_p = 1;
  shimm = 0;
  return insn;
}

/* Called at the end of processing normal insns (eg: add) to insert a shimm
   value (if present) into the insn.  */

static arc_insn
insert_shimmfinish (insn, operand, mods, reg, value, errmsg)
     arc_insn insn;
     const struct arc_operand *operand;
     int mods;
     const struct arc_operand_value *reg;
     long value;
     const char **errmsg;
{
  if (shimm_p)
    insn |= (shimm & ((1 << operand->bits) - 1)) << operand->shift;
  return insn;
}

/* Called at the end of processing normal insns (eg: add) to insert a limm
   value (if present) into the insn.

   Note that this function is only intended to handle instructions (with 4 byte
   immediate operands).  It is not intended to handle data.  */

/* ??? Actually, there's nothing for us to do as we can't call frag_more, the
   caller must do that.  The extract fns take a pointer to two words.  The
   insert fns could be converted and then we could do something useful, but
   then the reloc handlers would have to know to work on the second word of
   a 2 word quantity.  That's too much so we don't handle them.  */

static arc_insn
insert_limmfinish (insn, operand, mods, reg, value, errmsg)
     arc_insn insn;
     const struct arc_operand *operand;
     int mods;
     const struct arc_operand_value *reg;
     long value;
     const char **errmsg;
{
  if (limm_p)
    /* FIXME: put an abort here and see what happens.  */
    ; /* nothing to do, gas does it */
  return insn;
}

/* Called at the end of unary operand macros to copy the B field to C.  */

static arc_insn
insert_unopmacro (insn, operand, mods, reg, value, errmsg)
     arc_insn insn;
     const struct arc_operand *operand;
     int mods;
     const struct arc_operand_value *reg;
     long value;
     const char **errmsg;
{
  insn |= ((insn >> ARC_SHIFT_REGB) & ARC_MASK_REG) << operand->shift;
  return insn;
}

/* Insert a relative address for a branch insn (b, bl, or lp).  */

static arc_insn
insert_reladdr (insn, operand, mods, reg, value, errmsg)
     arc_insn insn;
     const struct arc_operand *operand;
     int mods;
     const struct arc_operand_value *reg;
     long value;
     const char **errmsg;
{
  if (value & 3)
    *errmsg = "branch address not on 4 byte boundary";
  insn |= ((value >> 2) & ((1 << operand->bits) - 1)) << operand->shift;
  return insn;
}

/* Insert a limm value as a 26 bit address right shifted 2 into the insn.

   Note that this function is only intended to handle instructions (with 4 byte
   immediate operands).  It is not intended to handle data.  */

/* ??? Actually, there's nothing for us to do as we can't call frag_more, the
   caller must do that.  The extract fns take a pointer to two words.  The
   insert fns could be converted and then we could do something useful, but
   then the reloc handlers would have to know to work on the second word of
   a 2 word quantity.  That's too much so we don't handle them.  */

static arc_insn
insert_absaddr (insn, operand, mods, reg, value, errmsg)
     arc_insn insn;
     const struct arc_operand *operand;
     int mods;
     const struct arc_operand_value *reg;
     long value;
     const char **errmsg;
{
  if (limm_p)
    /* FIXME: put an abort here and see what happens.  */
    ; /* nothing to do */
  return insn;
}

/* Extraction functions.

   The suffix extraction functions' return value is redundant since it can be
   obtained from (*OPVAL)->value.  However, the boolean suffixes don't have
   a suffix table entry for the "false" case, so values of zero must be
   obtained from the return value (*OPVAL == NULL).  */

static const struct arc_operand_value *lookup_register (int type, long regno);

/* Called by the disassembler before printing an instruction.  */

void
arc_opcode_init_extract ()
{
  flag_p = 0;
  flagshimm_handled_p = 0;
  shimm_p = 0;
  limm_p = 0;
}

/* As we're extracting registers, keep an eye out for the 'f' indicator
   (ARC_REG_SHIMM_UPDATE).  If we find a register (not a constant marker,
   like ARC_REG_SHIMM), set OPVAL so our caller will know this is a register.

   We must also handle auxiliary registers for lr/sr insns.  They are just
   constants with special names.  */

static long
extract_reg (insn, operand, mods, opval, invalid)
     arc_insn *insn;
     const struct arc_operand *operand;
     int mods;
     const struct arc_operand_value **opval;
     int *invalid;
{
  int regno;
  long value;

  /* Get the register number.  */
  regno = (insn[0] >> operand->shift) & ((1 << operand->bits) - 1);

  /* Is it a constant marker?  */
  if (regno == ARC_REG_SHIMM)
    {
      value = insn[0] & 511;
      if ((operand->flags & ARC_OPERAND_SIGNED)
	  && (value & 256))
	value -= 512;
      flagshimm_handled_p = 1;
    }
  else if (regno == ARC_REG_SHIMM_UPDATE)
    {
      value = insn[0] & 511;
      if ((operand->flags & ARC_OPERAND_SIGNED)
	  && (value & 256))
	value -= 512;
      flag_p = 1;
      flagshimm_handled_p = 1;
    }
  else if (regno == ARC_REG_LIMM)
    {
      value = insn[1];
      limm_p = 1;
    }
  /* It's a register, set OPVAL (that's the only way we distinguish registers
     from constants here).  */
  else
    {
      const struct arc_operand_value *reg = lookup_register (REG, regno);

      if (!reg)
	abort ();
      if (opval)
	*opval = reg;
      value = regno;
    }

  /* If this field takes an auxiliary register, see if it's a known one.  */
  if ((mods & ARC_MOD_AUXREG)
      && ARC_REG_CONSTANT_P (regno))
    {
      const struct arc_operand_value *reg = lookup_register (AUXREG, value);

      /* This is really a constant, but tell the caller it has a special
	 name.  */
      if (reg && opval)
	*opval = reg;
    }

  return value;
}

/* Return the value of the "flag update" field for shimm insns.
   This value is actually stored in the register field.  */

static long
extract_flag (insn, operand, mods, opval, invalid)
     arc_insn *insn;
     const struct arc_operand *operand;
     int mods;
     const struct arc_operand_value **opval;
     int *invalid;
{
  int f;
  const struct arc_operand_value *val;

  if (flagshimm_handled_p)
    f = flag_p != 0;
  else
    f = (insn[0] & (1 << operand->shift)) != 0;

  /* There is no text for zero values.  */
  if (f == 0)
    return 0;

  val = arc_opcode_lookup_suffix (operand, 1);
  if (opval && val)
    *opval = val;
  return val->value;
}

/* Extract the condition code (if it exists).
   If we've seen a shimm value in this insn (meaning that the insn can't have
   a condition code field), then we don't store anything in OPVAL and return
   zero.  */

static long
extract_cond (insn, operand, mods, opval, invalid)
     arc_insn *insn;
     const struct arc_operand *operand;
     int mods;
     const struct arc_operand_value **opval;
     int *invalid;
{
  long cond;
  const struct arc_operand_value *val;

  if (flagshimm_handled_p)
    return 0;

  cond = (insn[0] >> operand->shift) & ((1 << operand->bits) - 1);
  val = arc_opcode_lookup_suffix (operand, cond);

  /* Ignore NULL values of `val'.  Several condition code values are
     reserved for extensions.  */
  if (opval && val)
    *opval = val;
  return cond;
}

/* Extract a branch address.
   We return the value as a real address (not right shifted by 2).  */

static long
extract_reladdr (insn, operand, mods, opval, invalid)
     arc_insn *insn;
     const struct arc_operand *operand;
     int mods;
     const struct arc_operand_value **opval;
     int *invalid;
{
  long addr;

  addr = (insn[0] >> operand->shift) & ((1 << operand->bits) - 1);
  if ((operand->flags & ARC_OPERAND_SIGNED)
      && (addr & (1 << (operand->bits - 1))))
    addr -= 1 << operand->bits;

  return addr << 2;
}

/* The only thing this does is set the `invalid' flag if B != C.
   This is needed because the "mov" macro appears before it's real insn "and"
   and we don't want the disassembler to confuse them.  */

static long
extract_unopmacro (insn, operand, mods, opval, invalid)
     arc_insn *insn;
     const struct arc_operand *operand;
     int mods;
     const struct arc_operand_value **opval;
     int *invalid;
{
  /* ??? This misses the case where B == ARC_REG_SHIMM_UPDATE &&
     C == ARC_REG_SHIMM (or vice versa).  No big deal.  Those insns will get
     printed as "and"s.  */
  if (((insn[0] >> ARC_SHIFT_REGB) & ARC_MASK_REG)
      != ((insn[0] >> ARC_SHIFT_REGC) & ARC_MASK_REG))
    if (invalid)
      *invalid = 1;

  return 0;
}

/* Utility for the extraction functions to return the index into
   `arc_suffixes'.  */

const struct arc_operand_value *
arc_opcode_lookup_suffix (type, value)
     const struct arc_operand *type;
     int value;
{
  register const struct arc_operand_value *v,*end;

  /* ??? This is a little slow and can be speeded up.  */

  for (v = arc_suffixes, end = arc_suffixes + arc_suffixes_count; v < end; ++v)
    if (type == &arc_operands[v->type]
	&& value == v->value)
      return v;
  return 0;
}

static const struct arc_operand_value *
lookup_register (type, regno)
     int type;
     long regno;
{
  register const struct arc_operand_value *r,*end;

  if (type == REG)
    return &arc_reg_names[regno];

  /* ??? This is a little slow and can be speeded up.  */

  for (r = arc_reg_names, end = arc_reg_names + arc_reg_names_count;
       r < end; ++r)
    if (type == r->type	&& regno == r->value)
      return r;
  return 0;
}
