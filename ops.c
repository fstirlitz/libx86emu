/****************************************************************************
*
*						Realmode X86 Emulator Library
*
*            	Copyright (C) 1996-1999 SciTech Software, Inc.
* 				     Copyright (C) David Mosberger-Tang
* 					   Copyright (C) 1999 Egbert Eich
*
*  ========================================================================
*
*  Permission to use, copy, modify, distribute, and sell this software and
*  its documentation for any purpose is hereby granted without fee,
*  provided that the above copyright notice appear in all copies and that
*  both that copyright notice and this permission notice appear in
*  supporting documentation, and that the name of the authors not be used
*  in advertising or publicity pertaining to distribution of the software
*  without specific, written prior permission.  The authors makes no
*  representations about the suitability of this software for any purpose.
*  It is provided "as is" without express or implied warranty.
*
*  THE AUTHORS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
*  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
*  EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
*  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
*  USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
*  OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
*  PERFORMANCE OF THIS SOFTWARE.
*
*  ========================================================================
*
* Language:		ANSI C
* Environment:	Any
* Developer:    Kendall Bennett
*
* Description:  This file includes subroutines to implement the decoding
*               and emulation of all the x86 processor instructions.
*
* There are approximately 250 subroutines in here, which correspond
* to the 256 byte-"opcodes" found on the 8086.  The table which
* dispatches this is found in the files optab.[ch].
*
* Each opcode proc has a comment preceeding it which gives it's table
* address.  Several opcodes are missing (undefined) in the table.
*
* Each proc includes information for decoding (OP_DECODE).
*
* Many of the procedures are *VERY* similar in coding.  This has
* allowed for a very large amount of code to be generated in a fairly
* short amount of time (i.e. cut, paste, and modify).  The result is
* that much of the code below could have been folded into subroutines
* for a large reduction in size of this file.  The downside would be
* that there would be a penalty in execution speed.  The file could
* also have been *MUCH* larger by inlining certain functions which
* were called.  This could have resulted even faster execution.  The
* prime directive I used to decide whether to inline the code or to
* modularize it, was basically: 1) no unnecessary subroutine calls,
* 2) no routines more than about 200 lines in size, and 3) modularize
* any code that I might not get right the first time.  The fetch_*
* subroutines fall into the latter category.  The The decode_* fall
* into the second category.  The coding of the "switch(mod){ .... }"
* in many of the subroutines below falls into the first category.
* Especially, the coding of {add,and,or,sub,...}_{byte,word}
* subroutines are an especially glaring case of the third guideline.
* Since so much of the code is cloned from other modules (compare
* opcode #00 to opcode #01), making the basic operations subroutine
* calls is especially important; otherwise mistakes in coding an
* "add" would represent a nightmare in maintenance.
*
****************************************************************************/


#include "include/x86emu_int.h"

/*----------------------------- Implementation ----------------------------*/


static u8 (*op_A_byte[8])(u8 d, u8 s) = {
  add_byte, or_byte, adc_byte, sbb_byte, and_byte, sub_byte, xor_byte, cmp_byte
};

static u16 (*op_A_word[8])(u16 d, u16 s) = {
  add_word, or_word, adc_word, sbb_word, and_word, sub_word, xor_word, cmp_word
};

static u32 (*op_A_long[8])(u32 d, u32 s) = {
  add_long, or_long, adc_long, sbb_long, and_long, sub_long, xor_long, cmp_long
};

static void decode_op_A(int type)
{
  switch(type) {
    case 0:
      OP_DECODE("add ");
      break;
    case 1:
      OP_DECODE("or ");
      break;
    case 2:
      OP_DECODE("adc ");
      break;
    case 3:
      OP_DECODE("sbb ");
      break;
    case 4:
      OP_DECODE("and ");
      break;
    case 5:
      OP_DECODE("sub ");
      break;
    case 6:
      OP_DECODE("xor ");
      break;
    case 7:
      OP_DECODE("cmp ");
      break;
  }
}


static u8 (*op_B_byte[8])(u8 d, u8 s) = {
  rol_byte, ror_byte, rcl_byte, rcr_byte, shl_byte, shr_byte, shl_byte, sar_byte
};

static u16 (*op_B_word[8])(u16 s, u8 d) = {
  rol_word, ror_word, rcl_word, rcr_word, shl_word, shr_word, shl_word, sar_word
};

static u32 (*op_B_long[8])(u32 s, u8 d) = {
  rol_long, ror_long, rcl_long, rcr_long, shl_long, shr_long, shl_long, sar_long
};

static void decode_op_B(int type)
{
  switch(type) {
    case 0:
      OP_DECODE("rol ");
      break;
    case 1:
      OP_DECODE("ror ");
      break;
    case 2:
      OP_DECODE("rcl ");
      break;
    case 3:
      OP_DECODE("rcr ");
      break;
    case 4:
      OP_DECODE("shl ");
      break;
    case 5:
      OP_DECODE("shr ");
      break;
    case 6:
      OP_DECODE("sal ");
      break;
    case 7:
      OP_DECODE("sar ");
      break;
  }
}


void decode_cond(int type)
{
  switch(type) {
    case 0:
      OP_DECODE("o ");
      break;
    case 1:
      OP_DECODE("no ");
      break;
    case 2:
      OP_DECODE("b ");
      break;
    case 3:
      OP_DECODE("nb ");
      break;
    case 4:
      OP_DECODE("z ");
      break;
    case 5:
      OP_DECODE("nz ");
      break;
    case 6:
      OP_DECODE("na ");
      break;
    case 7:
      OP_DECODE("a ");
      break;
    case 8:
      OP_DECODE("s ");
      break;
    case 9:
      OP_DECODE("ns ");
      break;
    case 10:
      OP_DECODE("p ");
      break;
    case 11:
      OP_DECODE("np ");
      break;
    case 12:
      OP_DECODE("l ");
      break;
    case 13:
      OP_DECODE("nl ");
      break;
    case 14:
      OP_DECODE("ng ");
      break;
    case 15:
      OP_DECODE("g ");
      break;
  }
}


/****************************************************************************
PARAMETERS:
op1 - Instruction op code

REMARKS:
Handles illegal opcodes.
****************************************************************************/
static void x86emuOp_illegal_op(u8 op1)
{
  OP_DECODE("illegal opcode");
  INTR_RAISE_UD(&M);
}


/****************************************************************************
REMARKS:
Handles opcode 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38
****************************************************************************/
static void x86emuOp_op_A_byte_RM_R(u8 op1)
{
  int mod, rl, rh, type;
  u8 *src, *dst, val;
  u32 addr;

  type = op1 >> 3;
  decode_op_A(type);

  fetch_decode_modrm(&mod, &rh, &rl);
  if(mod == 3) {
    dst = decode_rm_byte_register(rl);
    OP_DECODE(",");
    src = decode_rm_byte_register(rh);
    *dst = (*op_A_byte[type])(*dst, *src);
  }
  else {
    addr = decode_rm_address(mod, rl);
    OP_DECODE(",");
    val = fetch_data_byte(addr);
    src = decode_rm_byte_register(rh);
    val = (*op_A_byte[type])(val, *src);
    if(type != 7) store_data_byte(addr, val);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x01, 0x09, 0x11, 0x19, 0x21, 0x29, 0x31, 0x39
****************************************************************************/
static void x86emuOp_op_A_word_RM_R(u8 op1)
{
  int mod, rl, rh, type;
  u32 *src32, *dst32, val, addr;
  u16 *src16, *dst16;

  type = op1 >> 3;
  decode_op_A(type);

  fetch_decode_modrm(&mod, &rh, &rl);
  if(mod == 3) {
    if(MODE_DATA32) {
      dst32 = decode_rm_long_register(rl);
      OP_DECODE(",");
      src32 = decode_rm_long_register(rh);
      *dst32 = (*op_A_long[type])(*dst32, *src32);
    }
    else {
      dst16 = decode_rm_word_register(rl);
      OP_DECODE(",");
      src16 = decode_rm_word_register(rh);
      *dst16 = (*op_A_word[type])(*dst16, *src16);
    }
  }
  else {
    addr = decode_rm_address(mod, rl);
    OP_DECODE(",");
    if(MODE_DATA32) {
      val = fetch_data_long(addr);
      src32 = decode_rm_long_register(rh);
      val = (*op_A_long[type])(val, *src32);
      if(type != 7) store_data_long(addr, val);
    }
    else {
      val = fetch_data_word(addr);
      src16 = decode_rm_word_register(rh);
      val = (*op_A_word[type])(val, *src16);
      if(type != 7) store_data_word(addr, val);
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x02, 0x0a, 0x12, 0x1a, 0x22, 0x2a, 0x32, 0x3a
****************************************************************************/
static void x86emuOp_op_A_byte_R_RM(u8 op1)
{
  int mod, rl, rh, type;
  u8 *src, *dst, val;
  u32 addr;

  type = op1 >> 3;
  decode_op_A(type);

  fetch_decode_modrm(&mod, &rh, &rl);
  if(mod == 3) {
    dst = decode_rm_byte_register(rh);
    OP_DECODE(",");
    src = decode_rm_byte_register(rl);
    *dst = (*op_A_byte[type])(*dst, *src);
  }
  else {
    dst = decode_rm_byte_register(rh);
    OP_DECODE(",");
    addr = decode_rm_address(mod, rl);
    val = fetch_data_byte(addr);
    *dst = (*op_A_byte[type])(*dst, val);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x03, 0x0b, 0x13, 0x1b, 0x23, 0x2b, 0x33, 0x3b
****************************************************************************/
static void x86emuOp_op_A_word_R_RM(u8 op1)
{
  int mod, rl, rh, type;
  u32 *src32, *dst32, val, addr;
  u16 *src16, *dst16;

  type = op1 >> 3;
  decode_op_A(type);

  fetch_decode_modrm(&mod, &rh, &rl);
  if(mod == 3) {
    if(MODE_DATA32) {
      dst32 = decode_rm_long_register(rh);
      OP_DECODE(",");
      src32 = decode_rm_long_register(rl);
      *dst32 = (*op_A_long[type])(*dst32, *src32);
    }
    else {
      dst16 = decode_rm_word_register(rh);
      OP_DECODE(",");
      src16 = decode_rm_word_register(rl);
      *dst16 = (*op_A_word[type])(*dst16, *src16);
    }
  }
  else {
    if(MODE_DATA32) {
      dst32 = decode_rm_long_register(rh);
      OP_DECODE(",");
      addr = decode_rm_address(mod, rl);
      val = fetch_data_long(addr);
      *dst32 = (*op_A_long[type])(*dst32, val);
    }
    else {
      dst16 = decode_rm_word_register(rh);
      OP_DECODE(",");
      addr = decode_rm_address(mod, rl);
      val = fetch_data_word(addr);
      *dst16 = (*op_A_word[type])(*dst16, val);
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x04, 0x0c, 0x14, 0x1c, 0x24, 0x2c, 0x34, 0x3c
****************************************************************************/
static void x86emuOp_op_A_byte_AL_IMM(u8 op1)
{
  int type;
  u8 val;

  type = op1 >> 3;
  decode_op_A(type);
  OP_DECODE("al,");

  val = fetch_byte();
  DECODE_HEX2(val);
  M.x86.R_AL = (*op_A_byte[type])(M.x86.R_AL, val);
}


/****************************************************************************
REMARKS:
Handles opcode 0x05, 0x0d, 0x15, 0x1d, 0x25, 0x2d, 0x35, 0x3d
****************************************************************************/
static void x86emuOp_op_A_word_AX_IMM(u8 op1)
{
  int type;
  u32 val;

  type = op1 >> 3;
  decode_op_A(type);

  if(MODE_DATA32) {
    OP_DECODE("eax,");
    val = fetch_long();
    DECODE_HEX8(val);
    M.x86.R_EAX = (*op_A_long[type])(M.x86.R_EAX, val);
  }
  else {
    OP_DECODE("ax,");
    val = fetch_word();
    DECODE_HEX4(val);
    M.x86.R_AX = (*op_A_word[type])(M.x86.R_AX, val);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x06
****************************************************************************/
static void x86emuOp_push_ES(u8 op1)
{
  OP_DECODE("push es");

  if(MODE_DATA32) {
    push_long(M.x86.R_ES);
  }
  else {
    push_word(M.x86.R_ES);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x07
****************************************************************************/
static void x86emuOp_pop_ES(u8 op1)
{
  OP_DECODE("pop es");
  x86emu_set_seg_register(&M, M.x86.R_ES_SEL, MODE_DATA32 ? pop_long() : pop_word());
}


/****************************************************************************
REMARKS:
Handles opcode 0x0e
****************************************************************************/
static void x86emuOp_push_CS(u8 op1)
{
  OP_DECODE("push cs");
  if(MODE_DATA32) {
    push_long(M.x86.R_CS);
  }
  else {
    push_word(M.x86.R_CS);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x0f. Escape for two-byte opcode (286 or better)
****************************************************************************/
static void x86emuOp_two_byte(u8 op1)
{
  u8 op2 = fetch_byte();

  (*x86emu_optab2[op2])(op2);
}


/****************************************************************************
REMARKS:
Handles opcode 0x16
****************************************************************************/
static void x86emuOp_push_SS(u8 op1)
{
  OP_DECODE("push ss");

  if(MODE_DATA32) {
    push_long(M.x86.R_SS);
  }
  else {
    push_word(M.x86.R_SS);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x17
****************************************************************************/
static void x86emuOp_pop_SS(u8 op1)
{
  OP_DECODE("pop ss");
  x86emu_set_seg_register(&M, M.x86.R_SS_SEL, MODE_DATA32 ? pop_long() : pop_word());
}


/****************************************************************************
REMARKS:
Handles opcode 0x1e
****************************************************************************/
static void x86emuOp_push_DS(u8 op1)
{
  OP_DECODE("push ds");

  if(MODE_DATA32) {
    push_long(M.x86.R_DS);
  }
  else {
    push_word(M.x86.R_DS);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x1f
****************************************************************************/
static void x86emuOp_pop_DS(u8 op1)
{
  OP_DECODE("pop ds");
  x86emu_set_seg_register(&M, M.x86.R_DS_SEL, MODE_DATA32 ? pop_long() : pop_word());
}


/****************************************************************************
REMARKS:
Handles opcode 0x27
****************************************************************************/
static void x86emuOp_daa(u8 op1)
{
  OP_DECODE("daa");
  M.x86.R_AL = daa_byte(M.x86.R_AL);
}


/****************************************************************************
REMARKS:
Handles opcode 0x2f
****************************************************************************/
static void x86emuOp_das(u8 op1)
{
  OP_DECODE("das");
  M.x86.R_AL = das_byte(M.x86.R_AL);
}


/****************************************************************************
REMARKS:
Handles opcode 0x37
****************************************************************************/
static void x86emuOp_aaa(u8 op1)
{
  OP_DECODE("aaa");
  M.x86.R_AX = aaa_word(M.x86.R_AX);
}


/****************************************************************************
REMARKS:
Handles opcode 0x3f
****************************************************************************/
static void x86emuOp_aas(u8 op1)
{
  OP_DECODE("aas");
  M.x86.R_AX = aas_word(M.x86.R_AX);
}


/****************************************************************************
REMARKS:
Handles opcode 0x40
****************************************************************************/
static void x86emuOp_inc_AX(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("inc eax");
    M.x86.R_EAX = inc_long(M.x86.R_EAX);
  }
  else {
    OP_DECODE("inc ax");
    M.x86.R_AX = inc_word(M.x86.R_AX);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x41
****************************************************************************/
static void x86emuOp_inc_CX(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("inc ecx");
    M.x86.R_ECX = inc_long(M.x86.R_ECX);
  }
  else {
    OP_DECODE("inc cx");
    M.x86.R_CX = inc_word(M.x86.R_CX);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x42
****************************************************************************/
static void x86emuOp_inc_DX(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("inc edx");
    M.x86.R_EDX = inc_long(M.x86.R_EDX);
  }
  else {
    OP_DECODE("inc dx");
    M.x86.R_DX = inc_word(M.x86.R_DX);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x43
****************************************************************************/
static void x86emuOp_inc_BX(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("inc ebx");
    M.x86.R_EBX = inc_long(M.x86.R_EBX);
  }
  else {
    OP_DECODE("inc bx");
    M.x86.R_BX = inc_word(M.x86.R_BX);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x44
****************************************************************************/
static void x86emuOp_inc_SP(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("inc esp");
    M.x86.R_ESP = inc_long(M.x86.R_ESP);
  }
  else {
    OP_DECODE("inc sp");
    M.x86.R_SP = inc_word(M.x86.R_SP);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x45
****************************************************************************/
static void x86emuOp_inc_BP(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("inc ebp");
    M.x86.R_EBP = inc_long(M.x86.R_EBP);
  }
  else {
    OP_DECODE("inc bp");
    M.x86.R_BP = inc_word(M.x86.R_BP);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x46
****************************************************************************/
static void x86emuOp_inc_SI(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("inc esi");
    M.x86.R_ESI = inc_long(M.x86.R_ESI);
  }
  else {
    OP_DECODE("inc si");
    M.x86.R_SI = inc_word(M.x86.R_SI);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x47
****************************************************************************/
static void x86emuOp_inc_DI(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("inc edi");
    M.x86.R_EDI = inc_long(M.x86.R_EDI);
  }
  else {
    OP_DECODE("inc di");
    M.x86.R_DI = inc_word(M.x86.R_DI);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x48
****************************************************************************/
static void x86emuOp_dec_AX(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("dec eax");
    M.x86.R_EAX = dec_long(M.x86.R_EAX);
  }
  else {
    OP_DECODE("dec ax");
    M.x86.R_AX = dec_word(M.x86.R_AX);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x49
****************************************************************************/
static void x86emuOp_dec_CX(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("dec ecx");
    M.x86.R_ECX = dec_long(M.x86.R_ECX);
  }
  else {
    OP_DECODE("dec cx");
    M.x86.R_CX = dec_word(M.x86.R_CX);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x4a
****************************************************************************/
static void x86emuOp_dec_DX(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("dec edx");
    M.x86.R_EDX = dec_long(M.x86.R_EDX);
  }
  else {
    OP_DECODE("dec dx");
    M.x86.R_DX = dec_word(M.x86.R_DX);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x4b
****************************************************************************/
static void x86emuOp_dec_BX(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("dec ebx");
    M.x86.R_EBX = dec_long(M.x86.R_EBX);
  }
  else {
    OP_DECODE("dec bx");
    M.x86.R_BX = dec_word(M.x86.R_BX);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x4c
****************************************************************************/
static void x86emuOp_dec_SP(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("dec esp");
    M.x86.R_ESP = dec_long(M.x86.R_ESP);
  }
  else {
    OP_DECODE("dec sp");
    M.x86.R_SP = dec_word(M.x86.R_SP);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x4d
****************************************************************************/
static void x86emuOp_dec_BP(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("dec ebp");
    M.x86.R_EBP = dec_long(M.x86.R_EBP);
  }
  else {
    OP_DECODE("dec bp");
    M.x86.R_BP = dec_word(M.x86.R_BP);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x4e
****************************************************************************/
static void x86emuOp_dec_SI(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("dec esi");
    M.x86.R_ESI = dec_long(M.x86.R_ESI);
  }
  else {
    OP_DECODE("dec si");
    M.x86.R_SI = dec_word(M.x86.R_SI);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x4f
****************************************************************************/
static void x86emuOp_dec_DI(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("dec edi");
    M.x86.R_EDI = dec_long(M.x86.R_EDI);
  }
  else {
    OP_DECODE("dec di");
    M.x86.R_DI = dec_word(M.x86.R_DI);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x50
****************************************************************************/
static void x86emuOp_push_AX(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("push eax");
    push_long(M.x86.R_EAX);
  }
  else {
    OP_DECODE("push ax");
    push_word(M.x86.R_AX);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x51
****************************************************************************/
static void x86emuOp_push_CX(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("push ecx");
    push_long(M.x86.R_ECX);
  }
  else {
    OP_DECODE("push cx");
    push_word(M.x86.R_CX);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x52
****************************************************************************/
static void x86emuOp_push_DX(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("push edx");
    push_long(M.x86.R_EDX);
  }
  else {
    OP_DECODE("push dx");
    push_word(M.x86.R_DX);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x53
****************************************************************************/
static void x86emuOp_push_BX(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("push ebx");
    push_long(M.x86.R_EBX);
  }
  else {
    OP_DECODE("push bx");
    push_word(M.x86.R_BX);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x54
****************************************************************************/
static void x86emuOp_push_SP(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("push esp");
    push_long(M.x86.R_ESP);
  }
  else {
    OP_DECODE("push sp");
    push_word(M.x86.R_SP);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x55
****************************************************************************/
static void x86emuOp_push_BP(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("push ebp");
    push_long(M.x86.R_EBP);
  }
  else {
    OP_DECODE("push bp");
    push_word(M.x86.R_BP);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x56
****************************************************************************/
static void x86emuOp_push_SI(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("push esi");
    push_long(M.x86.R_ESI);
  }
  else {
    OP_DECODE("push si");
    push_word(M.x86.R_SI);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x57
****************************************************************************/
static void x86emuOp_push_DI(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("push edi");
    push_long(M.x86.R_EDI);
  }
  else {
    OP_DECODE("push di");
    push_word(M.x86.R_DI);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x58
****************************************************************************/
static void x86emuOp_pop_AX(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("pop eax");
    M.x86.R_EAX = pop_long();
  }
  else {
    OP_DECODE("pop ax");
    M.x86.R_AX = pop_word();
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x59
****************************************************************************/
static void x86emuOp_pop_CX(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("pop ecx");
    M.x86.R_ECX = pop_long();
  }
  else {
    OP_DECODE("pop cx");
    M.x86.R_CX = pop_word();
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x5a
****************************************************************************/
static void x86emuOp_pop_DX(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("pop edx");
    M.x86.R_EDX = pop_long();
  }
  else {
    OP_DECODE("pop dx");
    M.x86.R_DX = pop_word();
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x5b
****************************************************************************/
static void x86emuOp_pop_BX(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("pop ebx");
    M.x86.R_EBX = pop_long();
  }
  else {
    OP_DECODE("pop bx");
    M.x86.R_BX = pop_word();
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x5c
****************************************************************************/
static void x86emuOp_pop_SP(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("pop esp");
    M.x86.R_ESP = pop_long();
  }
  else {
    OP_DECODE("pop sp");
    M.x86.R_SP = pop_word();
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x5d
****************************************************************************/
static void x86emuOp_pop_BP(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("pop ebp");
    M.x86.R_EBP = pop_long();
  }
  else {
    OP_DECODE("pop bp");
    M.x86.R_BP = pop_word();
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x5e
****************************************************************************/
static void x86emuOp_pop_SI(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("pop esi");
    M.x86.R_ESI = pop_long();
  }
  else {
    OP_DECODE("pop si");
    M.x86.R_SI = pop_word();
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x5f
****************************************************************************/
static void x86emuOp_pop_DI(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("pop edi");
    M.x86.R_EDI = pop_long();
  }
  else {
    OP_DECODE("pop di");
    M.x86.R_DI = pop_word();
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x60
****************************************************************************/
static void x86emuOp_push_all(u8 op1)
{
  u32 esp = M.x86.R_ESP;

  if(MODE_DATA32) {
    OP_DECODE("pushad");

    push_long(M.x86.R_EAX);
    push_long(M.x86.R_ECX);
    push_long(M.x86.R_EDX);
    push_long(M.x86.R_EBX);
    push_long(esp);
    push_long(M.x86.R_EBP);
    push_long(M.x86.R_ESI);
    push_long(M.x86.R_EDI);
  }
  else {
    OP_DECODE("pusha");

    push_word(M.x86.R_AX);
    push_word(M.x86.R_CX);
    push_word(M.x86.R_DX);
    push_word(M.x86.R_BX);
    push_word(esp);
    push_word(M.x86.R_BP);
    push_word(M.x86.R_SI);
    push_word(M.x86.R_DI);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x61
****************************************************************************/
static void x86emuOp_pop_all(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("popad");

    M.x86.R_EDI = pop_long();
    M.x86.R_ESI = pop_long();
    M.x86.R_EBP = pop_long();
    M.x86.R_ESP += 4;
    M.x86.R_EBX = pop_long();
    M.x86.R_EDX = pop_long();
    M.x86.R_ECX = pop_long();
    M.x86.R_EAX = pop_long();
  }
  else {
    OP_DECODE("popa");

    M.x86.R_DI = pop_word();
    M.x86.R_SI = pop_word();
    M.x86.R_BP = pop_word();
    M.x86.R_SP += 2;
    M.x86.R_BX = pop_word();
    M.x86.R_DX = pop_word();
    M.x86.R_CX = pop_word();
    M.x86.R_AX = pop_word();
  }
}


/* opcode 0x62: BOUND (not implemented) */
/* opcode 0x63: ARPL (not implemented) */


/****************************************************************************
REMARKS:
Handles opcode 0x68
****************************************************************************/
static void x86emuOp_push_word_IMM(u8 op1)
{
  u32 imm;

  OP_DECODE("push ");

  if(MODE_DATA32) {
    imm = fetch_long();
    DECODE_HEX8(imm);
    push_long(imm);
  }
  else {
    imm = fetch_word();
    DECODE_HEX4(imm);
    push_word(imm);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x69
****************************************************************************/
static void x86emuOp_imul_word_IMM(u8 op1)
{
  int mod, rl, rh;
  s32 imm;
  u32 *src32, *dst32, val, addr, res_lo, res_hi;
  u16 *src16, *dst16;

  OP_DECODE("imul ");
  fetch_decode_modrm(&mod, &rh, &rl);
  if(mod == 3) {
    if(MODE_DATA32) {
      dst32 = decode_rm_long_register(rh);
      OP_DECODE(",");
      src32 = decode_rm_long_register(rl);
      imm = fetch_long();
      OP_DECODE(",");
      DECODE_HEX8S(imm);
      imul_long_direct(&res_lo, &res_hi, *src32, imm);
      if(res_hi != 0) {
        SET_FLAG(F_CF);
        SET_FLAG(F_OF);
      }
      else {
        CLEAR_FLAG(F_CF);
        CLEAR_FLAG(F_OF);
      }
      *dst32 = res_lo;
    }
    else {
      dst16 = decode_rm_word_register(rh);
      OP_DECODE(",");
      src16 = decode_rm_word_register(rl);
      imm = (s16) fetch_word();
      OP_DECODE(",");
      DECODE_HEX4S(imm);
      res_lo = (s32) ((s16) *src16 * imm);
      if(res_lo > 0xffff) {
        SET_FLAG(F_CF);
        SET_FLAG(F_OF);
      }
      else {
        CLEAR_FLAG(F_CF);
        CLEAR_FLAG(F_OF);
      }
      *dst16 = res_lo;
    }
  }
  else {
    if(MODE_DATA32) {
      dst32 = decode_rm_long_register(rh);
      OP_DECODE(",");
      addr = decode_rm_address(mod, rl);
      val = fetch_data_long(addr);
      imm = fetch_long();
      OP_DECODE(",");
      DECODE_HEX8S(imm);
      imul_long_direct(&res_lo, &res_hi, val, imm);
      if(res_hi != 0) {
        SET_FLAG(F_CF);
        SET_FLAG(F_OF);
      }
      else {
        CLEAR_FLAG(F_CF);
        CLEAR_FLAG(F_OF);
      }
      *dst32 = res_lo;
    }
    else {
      dst16 = decode_rm_word_register(rh);
      OP_DECODE(",");
      addr = decode_rm_address(mod, rl);
      val = fetch_data_word(addr);
      imm = (s32) fetch_word();
      OP_DECODE(",");
      DECODE_HEX4S(imm);
      res_lo = (s32) ((s16) val * imm);
      if(res_lo > 0xffff) {
        SET_FLAG(F_CF);
        SET_FLAG(F_OF);
      }
      else {
        CLEAR_FLAG(F_CF);
        CLEAR_FLAG(F_OF);
      }
      *dst16 = res_lo;
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x6a
****************************************************************************/
static void x86emuOp_push_byte_IMM(u8 op1)
{
  s32 imm;

  OP_DECODE("push ");

  imm = (s8) fetch_byte();
  DECODE_HEX2S(imm);
  if(MODE_DATA32) {
    push_long(imm);
  }
  else {
    push_word(imm);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x6b
****************************************************************************/
static void x86emuOp_imul_byte_IMM(u8 op1)
{
  int mod, rl, rh;
  s32 imm;
  u32 *src32, *dst32, val, addr, res_lo, res_hi;
  u16 *src16, *dst16;

  OP_DECODE("imul ");
  fetch_decode_modrm(&mod, &rh, &rl);
  if(mod == 3) {
    if(MODE_DATA32) {
      dst32 = decode_rm_long_register(rh);
      OP_DECODE(",");
      src32 = decode_rm_long_register(rl);
      imm = (s8) fetch_byte();
      OP_DECODE(",");
      DECODE_HEX2S(imm);
      imul_long_direct(&res_lo, &res_hi, *src32, imm);
      if(res_hi != 0) {
        SET_FLAG(F_CF);
        SET_FLAG(F_OF);
      }
      else {
        CLEAR_FLAG(F_CF);
        CLEAR_FLAG(F_OF);
      }
      *dst32 = res_lo;
    }
    else {
      dst16 = decode_rm_word_register(rh);
      OP_DECODE(",");
      src16 = decode_rm_word_register(rl);
      imm = (s8) fetch_byte();
      OP_DECODE(",");
      DECODE_HEX2S(imm);
      res_lo = (s32) ((s16) *src16 * imm);
      if(res_lo > 0xffff) {
        SET_FLAG(F_CF);
        SET_FLAG(F_OF);
      }
      else {
        CLEAR_FLAG(F_CF);
        CLEAR_FLAG(F_OF);
      }
      *dst16 = res_lo;
    }
  }
  else {
    if(MODE_DATA32) {
      dst32 = decode_rm_long_register(rh);
      OP_DECODE(",");
      addr = decode_rm_address(mod, rl);
      val = fetch_data_long(addr);
      imm = (s8) fetch_byte();
      OP_DECODE(",");
      DECODE_HEX2S(imm);
      imul_long_direct(&res_lo, &res_hi, val, imm);
      if(res_hi != 0) {
        SET_FLAG(F_CF);
        SET_FLAG(F_OF);
      }
      else {
        CLEAR_FLAG(F_CF);
        CLEAR_FLAG(F_OF);
      }
      *dst32 = res_lo;
    }
    else {
      dst16 = decode_rm_word_register(rh);
      OP_DECODE(",");
      addr = decode_rm_address(mod, rl);
      val = fetch_data_word(addr);
      imm = (s8) fetch_byte();
      OP_DECODE(",");
      DECODE_HEX2S(imm);
      res_lo = (s32) ((s16) val * imm);
      if(res_lo > 0xffff) {
        SET_FLAG(F_CF);
        SET_FLAG(F_OF);
      }
      else {
        CLEAR_FLAG(F_CF);
        CLEAR_FLAG(F_OF);
      }
      *dst16 = res_lo;
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x6c
****************************************************************************/
static void x86emuOp_ins_byte(u8 op1)
{
  if((MODE_ADDR32 && !MODE_CODE32) || (!MODE_ADDR32 && MODE_CODE32)) {	/* xor */
    OP_DECODE("a32 ");
  }

  OP_DECODE("insb");
  ins(1);
}


/****************************************************************************
REMARKS:
Handles opcode 0x6d
****************************************************************************/
static void x86emuOp_ins_word(u8 op1)
{
  if((MODE_ADDR32 && !MODE_CODE32) || (!MODE_ADDR32 && MODE_CODE32)) {	/* xor */
    OP_DECODE("a32 ");
  }

  if(MODE_DATA32) {
    OP_DECODE("insd");
    ins(4);
  }
  else {
    OP_DECODE("insw");
    ins(2);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x6e
****************************************************************************/
static void x86emuOp_outs_byte(u8 op1)
{
  if((MODE_ADDR32 && !MODE_CODE32) || (!MODE_ADDR32 && MODE_CODE32)) {	/* xor */
    OP_DECODE("a32 ");
  }

  OP_DECODE("outsb");
  outs(1);
}


/****************************************************************************
REMARKS:
Handles opcode 0x6f
****************************************************************************/
static void x86emuOp_outs_word(u8 op1)
{
  if((MODE_ADDR32 && !MODE_CODE32) || (!MODE_ADDR32 && MODE_CODE32)) {	/* xor */
    OP_DECODE("a32 ");
  }

  if(MODE_DATA32) {
    OP_DECODE("outsd");
    outs(4);
  }
  else {
    OP_DECODE("outsw");
    outs(2);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x70-0x7F
****************************************************************************/
static void x86emuOp_jump_short_cc(u8 op1)
{
  s32 ofs;
  u32 eip;
  unsigned type = op1 & 0xf;

  OP_DECODE("j");
  decode_cond(type);

  ofs = (s8) fetch_byte();
  eip = M.x86.R_EIP + ofs;
  DECODE_HEX_ADDR(eip);
  if(!MODE_DATA32) eip &= 0xffff;
  if(eval_condition(type)) M.x86.R_EIP = eip;
}


/****************************************************************************
REMARKS:
Handles opcode 0x80
****************************************************************************/
static void x86emuOp_opc80_byte_RM_IMM(u8 op1)
{
  int mod, rl, rh;
  u8 *reg8, val, imm;
  u32 addr;

  fetch_decode_modrm(&mod, &rh, &rl);

  decode_op_A(rh);

  if(mod == 3) {
    reg8 = decode_rm_byte_register(rl);
    OP_DECODE(",");
    imm = fetch_byte();
    DECODE_HEX2(imm);
    val = (*op_A_byte[rh])(*reg8, imm);
    if(rh != 7) *reg8 = val;
  }
  else {
    OP_DECODE("byte ");
    addr = decode_rm_address(mod, rl);
    OP_DECODE(",");
    val = fetch_data_byte(addr);
    imm = fetch_byte();
    DECODE_HEX2(imm);
    val = (*op_A_byte[rh])(val, imm);
    if(rh != 7) store_data_byte(addr, val);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x81
****************************************************************************/
static void x86emuOp_opc81_word_RM_IMM(u8 op1)
{
  int mod, rl, rh;
  u16 *reg16;
  u32 *reg32, val, imm, addr;

  fetch_decode_modrm(&mod, &rh, &rl);

  decode_op_A(rh);

  if(mod == 3) {
    if(MODE_DATA32) {
      reg32 = decode_rm_long_register(rl);
      OP_DECODE(",");
      imm = fetch_long();
      DECODE_HEX8(imm);
      val = (*op_A_long[rh])(*reg32, imm);
      if(rh != 7) *reg32 = val;
    }
    else {
      reg16 = decode_rm_word_register(rl);
      OP_DECODE(",");
      imm = fetch_word();
      DECODE_HEX4(imm);
      val = (*op_A_word[rh])(*reg16, imm);
      if(rh != 7) *reg16 = val;
    }
  }
  else {
    if(MODE_DATA32) {
      OP_DECODE("dword ");
      addr = decode_rm_address(mod, rl);
      OP_DECODE(",");
      val = fetch_data_long(addr);
      imm = fetch_long();
      DECODE_HEX8(imm);
      val = (*op_A_long[rh])(val, imm);
      if(rh != 7) store_data_long(addr, val);
    }
    else {
      OP_DECODE("word ");
      addr = decode_rm_address(mod, rl);
      OP_DECODE(",");
      val = fetch_data_word(addr);
      imm = fetch_word();
      DECODE_HEX4(imm);
      val = (*op_A_word[rh])(val, imm);
      if(rh != 7) store_data_word(addr, val);
    }
  }
}


/* opcode 0x82: same as 0x80 */


/****************************************************************************
REMARKS:
Handles opcode 0x83
****************************************************************************/
static void x86emuOp_opc83_word_RM_IMM(u8 op1)
{
  int mod, rl, rh;
  u16 *reg16;
  u32 *reg32, val, imm, addr;

  fetch_decode_modrm(&mod, &rh, &rl);

  decode_op_A(rh);

  if(mod == 3) {
    if(MODE_DATA32) {
      reg32 = decode_rm_long_register(rl);
      OP_DECODE(",");
      imm = (s8) fetch_byte();
      DECODE_HEX2S(imm);
      val = (*op_A_long[rh])(*reg32, imm);
      if(rh != 7) *reg32 = val;
    }
    else {
      reg16 = decode_rm_word_register(rl);
      OP_DECODE(",");
      imm = (s8) fetch_byte();
      DECODE_HEX2S(imm);
      val = (*op_A_word[rh])(*reg16, imm);
      if(rh != 7) *reg16 = val;
    }
  }
  else {
    if(MODE_DATA32) {
      OP_DECODE("dword ");
      addr = decode_rm_address(mod, rl);
      OP_DECODE(",");
      val = fetch_data_long(addr);
      imm = (s8) fetch_byte();
      DECODE_HEX2S(imm);
      val = (*op_A_long[rh])(val, imm);
      if(rh != 7) store_data_long(addr, val);
    }
    else {
      OP_DECODE("word ");
      addr = decode_rm_address(mod, rl);
      OP_DECODE(",");
      val = fetch_data_word(addr);
      imm = (s8) fetch_byte();
      DECODE_HEX2S(imm);
      val = (*op_A_word[rh])(val, imm);
      if(rh != 7) store_data_word(addr, val);
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x84
****************************************************************************/
static void x86emuOp_test_byte_RM_R(u8 op1)
{
  int mod, rl, rh;
  u8 *dst, *src, val;
  u32 addr;

  OP_DECODE("test ");
  fetch_decode_modrm(&mod, &rh, &rl);

  if(mod == 3) {
    dst = decode_rm_byte_register(rl);
    OP_DECODE(",");
    src = decode_rm_byte_register(rh);
    test_byte(*dst, *src);
  }
  else {
    addr = decode_rm_address(mod, rl);
    OP_DECODE(",");
    val = fetch_data_byte(addr);
    src = decode_rm_byte_register(rh);
    test_byte(val, *src);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x85
****************************************************************************/
static void x86emuOp_test_word_RM_R(u8 op1)
{
  int mod, rl, rh;
  u16 *src16, *dst16;
  u32 *src32, *dst32, addr, val;

  OP_DECODE("test ");
  fetch_decode_modrm(&mod, &rh, &rl);

  if(mod == 3) {
    if(MODE_DATA32) {
      dst32 = decode_rm_long_register(rl);
      OP_DECODE(",");
      src32 = decode_rm_long_register(rh);
      test_long(*dst32, *src32);
    }
    else {
      dst16 = decode_rm_word_register(rl);
      OP_DECODE(",");
      src16 = decode_rm_word_register(rh);
      test_word(*dst16, *src16);
    }
  }
  else {
    addr = decode_rm_address(mod, rl);
    OP_DECODE(",");

    if(MODE_DATA32) {
      val = fetch_data_long(addr);
      src32 = decode_rm_long_register(rh);
      test_long(val, *src32);
    }
    else {
      val = fetch_data_word(addr);
      src16 = decode_rm_word_register(rh);
      test_word(val, *src16);
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x86
****************************************************************************/
static void x86emuOp_xchg_byte_RM_R(u8 op1)
{
  int mod, rl, rh;
  u8 *dst, *src, val;
  u32 addr;

  OP_DECODE("xchg ");
  fetch_decode_modrm(&mod, &rh, &rl);

  if(mod == 3) {
    dst = decode_rm_byte_register(rl);
    OP_DECODE(",");
    src = decode_rm_byte_register(rh);
    val = *src;
    *src = *dst;
    *dst = val;
  }
  else {
    addr = decode_rm_address(mod, rl);
    OP_DECODE(",");
    val = fetch_data_byte(addr);
    src = decode_rm_byte_register(rh);
    store_data_byte(addr, *src);
    *src = val;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x87
****************************************************************************/
static void x86emuOp_xchg_word_RM_R(u8 op1)
{
  int mod, rl, rh;
  u16 *src16, *dst16;
  u32 *src32, *dst32, addr, val;

  OP_DECODE("xchg ");
  fetch_decode_modrm(&mod, &rh, &rl);

  if(mod == 3) {
    if(MODE_DATA32) {
      dst32 = decode_rm_long_register(rl);
      OP_DECODE(",");
      src32 = decode_rm_long_register(rh);
      val = *src32;
      *src32 = *dst32;
      *dst32 = val;
    }
    else {
      dst16 = decode_rm_word_register(rl);
      OP_DECODE(",");
      src16 = decode_rm_word_register(rh);
      val = *src16;
      *src16 = *dst16;
      *dst16 = val;
    }
  }
  else {
    addr = decode_rm_address(mod, rl);
    OP_DECODE(",");

    if(MODE_DATA32) {
      val = fetch_data_long(addr);
      src32 = decode_rm_long_register(rh);
      store_data_long(addr, *src32);
      *src32 = val;
    }
    else {
      val = fetch_data_word(addr);
      src16 = decode_rm_word_register(rh);
      store_data_word(addr, *src16);
      *src16 = val;
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x88
****************************************************************************/
static void x86emuOp_mov_byte_RM_R(u8 op1)
{
  int mod, rl, rh;
  u8 *dst, *src;
  u32 addr;

  OP_DECODE("mov ");
  fetch_decode_modrm(&mod, &rh, &rl);

  if(mod == 3) {
    dst = decode_rm_byte_register(rl);
    OP_DECODE(",");
    src = decode_rm_byte_register(rh);
    *dst = *src;
  }
  else {
    addr = decode_rm_address(mod, rl);
    OP_DECODE(",");
    src = decode_rm_byte_register(rh);
    store_data_byte(addr, *src);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x89
****************************************************************************/
static void x86emuOp_mov_word_RM_R(u8 op1)
{
  int mod, rl, rh;
  u16 *src16, *dst16;
  u32 *src32, *dst32, addr;

  OP_DECODE("mov ");
  fetch_decode_modrm(&mod, &rh, &rl);

  if(mod == 3) {
    if(MODE_DATA32) {
      dst32 = decode_rm_long_register(rl);
      OP_DECODE(",");
      src32 = decode_rm_long_register(rh);
      *dst32 = *src32;
    }
    else {
      dst16 = decode_rm_word_register(rl);
      OP_DECODE(",");
      src16 = decode_rm_word_register(rh);
      *dst16 = *src16;
    }
  }
  else {
    addr = decode_rm_address(mod, rl);
    OP_DECODE(",");

    if(MODE_DATA32) {
      src32 = decode_rm_long_register(rh);
      store_data_long(addr, *src32);
    }
    else {
      src16 = decode_rm_word_register(rh);
      store_data_word(addr, *src16);
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x8a
****************************************************************************/
static void x86emuOp_mov_byte_R_RM(u8 op1)
{
  int mod, rl, rh;
  u8 *dst, *src;
  u32 addr;

  OP_DECODE("mov ");
  fetch_decode_modrm(&mod, &rh, &rl);

  if(mod == 3) {
    dst = decode_rm_byte_register(rh);
    OP_DECODE(",");
    src = decode_rm_byte_register(rl);
    *dst = *src;
  }
  else {
    dst = decode_rm_byte_register(rh);
    OP_DECODE(",");
    addr = decode_rm_address(mod, rl);
    *dst = fetch_data_byte(addr);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x8b
****************************************************************************/
static void x86emuOp_mov_word_R_RM(u8 op1)
{
  int mod, rl, rh;
  u16 *src16, *dst16;
  u32 *src32, *dst32, addr;

  OP_DECODE("mov ");
  fetch_decode_modrm(&mod, &rh, &rl);

  if(mod == 3) {
    if(MODE_DATA32) {
      dst32 = decode_rm_long_register(rh);
      OP_DECODE(",");
      src32 = decode_rm_long_register(rl);
      *dst32 = *src32;
    }
    else {
      dst16 = decode_rm_word_register(rh);
      OP_DECODE(",");
      src16 = decode_rm_word_register(rl);
      *dst16 = *src16;
    }
  }
  else {
    if(MODE_DATA32) {
      dst32 = decode_rm_long_register(rh);
      OP_DECODE(",");
      addr = decode_rm_address(mod, rl);
      *dst32 = fetch_data_long(addr);
    }
    else {
      dst16 = decode_rm_word_register(rh);
      OP_DECODE(",");
      addr = decode_rm_address(mod, rl);
      *dst16 = fetch_data_word(addr);
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x8c
****************************************************************************/
static void x86emuOp_mov_word_RM_SR(u8 op1)
{
  int mod, rl, rh;
  u16 *reg16, val;
  u32 *reg32, addr;

  OP_DECODE("mov ");
  fetch_decode_modrm(&mod, &rh, &rl);

  if(mod == 3) {	/* register */
    if(MODE_DATA32) {
      reg32 = decode_rm_long_register(rl);
      OP_DECODE(",");
      *reg32 = decode_rm_seg_register(rh)->sel;
    }
    else {
      reg16 = decode_rm_word_register(rl);
      OP_DECODE(",");
      *reg16 = decode_rm_seg_register(rh)->sel;
    }
  }
  else {		/* memory */
    addr = decode_rm_address(mod, rl);
    OP_DECODE(",");
    val = decode_rm_seg_register(rh)->sel;
    store_data_word(addr, val);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x8d
****************************************************************************/
static void x86emuOp_lea_word_R_M(u8 op1)
{
  int mod, rl, rh;
  u16 *reg16;
  u32 *reg32, addr;

  OP_DECODE("lea ");
  fetch_decode_modrm(&mod, &rh, &rl);

  if(mod == 3) {
    INTR_RAISE_UD(&M);
  }
  else {
    if(MODE_DATA32) {
      reg32 = decode_rm_long_register(rh);
      OP_DECODE(",");
      addr = decode_rm_address(mod, rl);
      *reg32 = addr;
    }
    else {
      reg16 = decode_rm_word_register(rh);
      OP_DECODE(",");
      addr = decode_rm_address(mod, rl);
      *reg16 = addr;
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x8e
****************************************************************************/
static void x86emuOp_mov_word_SR_RM(u8 op1)
{
  int mod, rl, rh;
  u16 val;
  sel_t *seg;

  OP_DECODE("mov ");
  fetch_decode_modrm(&mod, &rh, &rl);
  seg = decode_rm_seg_register(rh);
  OP_DECODE(",");

  if(mod == 3) {	/* register */
    val = *decode_rm_word_register(rl);
  }
  else {		/* memory */
    val = fetch_data_word(decode_rm_address(mod, rl));
  }

  x86emu_set_seg_register(&M, seg, val);
}


/****************************************************************************
REMARKS:
Handles opcode 0x8f
****************************************************************************/
static void x86emuOp_pop_RM(u8 op1)
{
  int mod, rl, rh;
  u16 *reg16;
  u32 *reg32, addr, val;

  OP_DECODE("pop ");
  fetch_decode_modrm(&mod, &rh, &rl);

  if(rh != 0) {
    INTR_RAISE_UD(&M);
    return;
  }

  if(mod == 3) {
    if(MODE_DATA32) {
      reg32 = decode_rm_long_register(rl);
      *reg32 = pop_long();
    }
    else {
      reg16 = decode_rm_word_register(rl);
      *reg16 = pop_word();
    }
  }
  else {
    addr = decode_rm_address(mod, rl);

    if(MODE_DATA32) {
      val = pop_long();
      store_data_long(addr, val);
    }
    else {
      val = pop_word();
      store_data_word(addr, val);
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x90
****************************************************************************/
static void x86emuOp_nop(u8 op1)
{
  OP_DECODE("nop");
}


/****************************************************************************
REMARKS:
Handles opcode 0x91
****************************************************************************/
static void x86emuOp_xchg_word_AX_CX(u8 op1)
{
  u32 val;

  if(MODE_DATA32) {
    OP_DECODE("xchg eax,ecx");

    val = M.x86.R_EAX;
    M.x86.R_EAX = M.x86.R_ECX;
    M.x86.R_ECX = val;
  }
  else {
    OP_DECODE("xchg ax,cx");

    val = M.x86.R_AX;
    M.x86.R_AX = M.x86.R_CX;
    M.x86.R_CX = val;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x92
****************************************************************************/
static void x86emuOp_xchg_word_AX_DX(u8 op1)
{
  u32 val;

  if(MODE_DATA32) {
    OP_DECODE("xchg eax,edx");

    val = M.x86.R_EAX;
    M.x86.R_EAX = M.x86.R_EDX;
    M.x86.R_EDX = val;
  }
  else {
    OP_DECODE("xchg ax,dx");

    val = M.x86.R_AX;
    M.x86.R_AX = M.x86.R_DX;
    M.x86.R_DX = val;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x93
****************************************************************************/
static void x86emuOp_xchg_word_AX_BX(u8 op1)
{
  u32 val;

  if(MODE_DATA32) {
    OP_DECODE("xchg eax,ebx");

    val = M.x86.R_EAX;
    M.x86.R_EAX = M.x86.R_EBX;
    M.x86.R_EBX = val;
  }
  else {
    OP_DECODE("xchg ax,bx");

    val = M.x86.R_AX;
    M.x86.R_AX = M.x86.R_BX;
    M.x86.R_BX = val;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x94
****************************************************************************/
static void x86emuOp_xchg_word_AX_SP(u8 op1)
{
  u32 val;

  if(MODE_DATA32) {
    OP_DECODE("xchg eax,esp");

    val = M.x86.R_EAX;
    M.x86.R_EAX = M.x86.R_ESP;
    M.x86.R_ESP = val;
  }
  else {
    OP_DECODE("xchg ax,sp");

    val = M.x86.R_AX;
    M.x86.R_AX = M.x86.R_SP;
    M.x86.R_SP = val;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x95
****************************************************************************/
static void x86emuOp_xchg_word_AX_BP(u8 op1)
{
  u32 val;

  if(MODE_DATA32) {
    OP_DECODE("xchg eax,ebp");

    val = M.x86.R_EAX;
    M.x86.R_EAX = M.x86.R_EBP;
    M.x86.R_EBP = val;
  }
  else {
    OP_DECODE("xchg ax,bp");

    val = M.x86.R_AX;
    M.x86.R_AX = M.x86.R_BP;
    M.x86.R_BP = val;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x96
****************************************************************************/
static void x86emuOp_xchg_word_AX_SI(u8 op1)
{
  u32 val;

  if(MODE_DATA32) {
    OP_DECODE("xchg eax,esi");

    val = M.x86.R_EAX;
    M.x86.R_EAX = M.x86.R_ESI;
    M.x86.R_ESI = val;
  }
  else {
    OP_DECODE("xchg ax,si");

    val = M.x86.R_AX;
    M.x86.R_AX = M.x86.R_SI;
    M.x86.R_SI = val;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x97
****************************************************************************/
static void x86emuOp_xchg_word_AX_DI(u8 op1)
{
  u32 val;

  if(MODE_DATA32) {
    OP_DECODE("xchg eax,edi");

    val = M.x86.R_EAX;
    M.x86.R_EAX = M.x86.R_EDI;
    M.x86.R_EDI = val;
  }
  else {
    OP_DECODE("xchg ax,di");

    val = M.x86.R_AX;
    M.x86.R_AX = M.x86.R_DI;
    M.x86.R_DI = val;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x98
****************************************************************************/
static void x86emuOp_cbw(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("cwde");

    M.x86.R_EAX = (s16) M.x86.R_AX;
  }
  else {
    OP_DECODE("cbw");

    M.x86.R_AX = (s8) M.x86.R_AL;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x99
****************************************************************************/
static void x86emuOp_cwd(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("cdq");

    M.x86.R_EDX = - (M.x86.R_EAX >> 31);
  }
  else {
    OP_DECODE("cwd");

    M.x86.R_DX = - (M.x86.R_AX >> 15);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x9a
****************************************************************************/
static void x86emuOp_call_far_IMM(u8 op1)
{
  u16 cs;
  u32 eip;

  OP_DECODE("call far ");
  eip = MODE_DATA32 ? fetch_long() : fetch_word();
  cs = fetch_word();

  DECODE_HEX4(cs);
  OP_DECODE(":");
  if(MODE_DATA32) {
    DECODE_HEX8(eip);

    push_long(M.x86.R_CS);
    push_long(M.x86.R_EIP);
  }
  else {
    DECODE_HEX4(eip);

    push_word(M.x86.R_CS);
    push_word(M.x86.R_IP);
  }

  x86emu_set_seg_register(&M, M.x86.R_CS_SEL, cs);
  M.x86.R_EIP = eip;
}


/****************************************************************************
REMARKS:
Handles opcode 0x9b
****************************************************************************/
static void x86emuOp_wait(u8 op1)
{
  OP_DECODE("wait");
}


/****************************************************************************
REMARKS:
Handles opcode 0x9c
****************************************************************************/
static void x86emuOp_pushf_word(u8 op1)
{
  u32 flags;

  /* clear out *all* bits not representing flags, and turn on real bits */
  flags = (M.x86.R_EFLG & F_MSK) | F_ALWAYS_ON;

  if(MODE_DATA32) {
    OP_DECODE("pushfd");

    push_long(flags);
  }
  else {
    OP_DECODE("pushf");

    push_word(flags);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x9d
****************************************************************************/
static void x86emuOp_popf_word(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("popfd");

    M.x86.R_EFLG = pop_long() | F_ALWAYS_ON;
  }
  else {
    OP_DECODE("popf");

    M.x86.R_FLG = pop_word() | F_ALWAYS_ON;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0x9e
****************************************************************************/
static void x86emuOp_sahf(u8 op1)
{
  OP_DECODE("sahf");

  M.x86.R_FLG &= 0xffffff00;
  M.x86.R_FLG |= (M.x86.R_AH | F_ALWAYS_ON) & 0xff;
}


/****************************************************************************
REMARKS:
Handles opcode 0x9f
****************************************************************************/
static void x86emuOp_lahf(u8 op1)
{
  OP_DECODE("lahf");

  M.x86.R_AH = M.x86.R_FLG | F_ALWAYS_ON;
}


/****************************************************************************
REMARKS:
Handles opcode 0xa0
****************************************************************************/
static void x86emuOp_mov_AL_M_IMM(u8 op1)
{
  u32 addr;

  OP_DECODE("mov al,[");

  if(MODE_ADDR32) {
    addr = fetch_long();
    DECODE_HEX8(addr);
  }
  else {
    addr = fetch_word();
    DECODE_HEX4(addr);
  }

  OP_DECODE("]");

  M.x86.R_AL = fetch_data_byte(addr);
}


/****************************************************************************
REMARKS:
Handles opcode 0xa1
****************************************************************************/
static void x86emuOp_mov_AX_M_IMM(u8 op1)
{
  u32 addr;

  if(MODE_DATA32) {
    OP_DECODE("mov eax,[");
  }
  else {
    OP_DECODE("mov ax,[");
  }

  if(MODE_ADDR32) {
    addr = fetch_long();
    DECODE_HEX8(addr);
  }
  else {
    addr = fetch_word();
    DECODE_HEX4(addr);
  }

  OP_DECODE("]");

  if(MODE_DATA32) {
    M.x86.R_EAX = fetch_data_long(addr);
  }
  else {
    M.x86.R_AX = fetch_data_word(addr);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xa2
****************************************************************************/
static void x86emuOp_mov_M_AL_IMM(u8 op1)
{
  u32 addr;

  OP_DECODE("mov [");

  if(MODE_ADDR32) {
    addr = fetch_long();
    DECODE_HEX8(addr);
  }
  else {
    addr = fetch_word();
    DECODE_HEX4(addr);
  }

  OP_DECODE("],al");

  store_data_byte(addr, M.x86.R_AL);
}


/****************************************************************************
REMARKS:
Handles opcode 0xa3
****************************************************************************/
static void x86emuOp_mov_M_AX_IMM(u8 op1)
{
  u32 addr;

  OP_DECODE("mov [");

  if(MODE_ADDR32) {
    addr = fetch_long();
    DECODE_HEX8(addr);
  }
  else {
    addr = fetch_word();
    DECODE_HEX4(addr);
  }

  if(MODE_DATA32) {
    OP_DECODE("],eax");
    store_data_long(addr, M.x86.R_EAX);
  }
  else {
    OP_DECODE("],ax");
    store_data_word(addr, M.x86.R_AX);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xa4
****************************************************************************/
static void x86emuOp_movs_byte(u8 op1)
{
  u8  val;
  u32 count;
  s32 inc;

  inc = ACCESS_FLAG(F_DF) ? -1 : 1;	/* direction */

  count = 1;

  if(MODE_ADDR32) {
    if(!MODE_CODE32) OP_DECODE("a32 ");
    OP_DECODE("movsb");

    if(MODE_REP) {
      count = M.x86.R_ECX;
      M.x86.R_ECX = 0;
    }

    while(count--) {
      val = fetch_data_byte(M.x86.R_ESI);
      store_data_byte_abs(M.x86.seg + R_ES_INDEX, M.x86.R_EDI, val);
      M.x86.R_ESI += inc;
      M.x86.R_EDI += inc;
    }
  }
  else {
    if(MODE_CODE32) OP_DECODE("a32 ");
    OP_DECODE("movsb");

    if(MODE_REP) {
      /* dont care whether REPE or REPNE */
      /* move them until CX is ZERO. */
      count = M.x86.R_CX;
      M.x86.R_CX = 0;
    }

    while(count--) {
      val = fetch_data_byte(M.x86.R_SI);
      store_data_byte_abs(M.x86.seg + R_ES_INDEX, M.x86.R_DI, val);
      M.x86.R_SI += inc;
      M.x86.R_DI += inc;
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xa5
****************************************************************************/
static void x86emuOp_movs_word(u8 op1)
{
  u32 val, count;
  s32 inc;

  if((MODE_ADDR32 && !MODE_CODE32) || (!MODE_ADDR32 && MODE_CODE32)) {	/* xor */
    OP_DECODE("a32 ");
  }

  if(MODE_DATA32) {
    OP_DECODE("movsd");
    inc = ACCESS_FLAG(F_DF) ? -4 : 4;
  }
  else {
    OP_DECODE("movsw");
    inc = ACCESS_FLAG(F_DF) ? -2 : 2;
  }

  count = 1;

  if(MODE_ADDR32) {
    if(MODE_REP) {
      count = M.x86.R_ECX;
      M.x86.R_ECX = 0;
    }

    while(count--) {
      if(MODE_DATA32) {
        val = fetch_data_long(M.x86.R_ESI);
        store_data_long_abs(M.x86.seg + R_ES_INDEX, M.x86.R_EDI, val);
      }
      else {
        val = fetch_data_word(M.x86.R_ESI);
        store_data_word_abs(M.x86.seg + R_ES_INDEX, M.x86.R_EDI, val);
      }
      M.x86.R_ESI += inc;
      M.x86.R_EDI += inc;
    }
  }
  else {
    if(MODE_REP) {
      /* dont care whether REPE or REPNE */
      /* move them until CX is ZERO. */
      count = M.x86.R_CX;
      M.x86.R_CX = 0;
    }

    while(count--) {
      if(MODE_DATA32) {
        val = fetch_data_long(M.x86.R_SI);
        store_data_long_abs(M.x86.seg + R_ES_INDEX, M.x86.R_DI, val);
      }
      else {
        val = fetch_data_word(M.x86.R_SI);
        store_data_word_abs(M.x86.seg + R_ES_INDEX, M.x86.R_DI, val);
      }
      M.x86.R_SI += inc;
      M.x86.R_DI += inc;
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xa6
****************************************************************************/
static void x86emuOp_cmps_byte(u8 op1)
{
  u8 val1, val2;
  s32 inc;
  unsigned cond;

  inc = ACCESS_FLAG(F_DF) ? -1 : 1;

  cond = MODE_REPE ? (2 << 1) + 1 /* NZ */ : (2 << 1) /* Z */;

  if(MODE_ADDR32) {
    if(!MODE_CODE32) OP_DECODE("a32 ");
    OP_DECODE("cmpsb");

    if(MODE_REP) {
      while(M.x86.R_ECX) {
        val1 = fetch_data_byte(M.x86.R_ESI);
        val2 = fetch_data_byte_abs(M.x86.seg + R_ES_INDEX, M.x86.R_EDI);
        cmp_byte(val1, val2);
        M.x86.R_ECX--;
        M.x86.R_ESI += inc;
        M.x86.R_EDI += inc;
        if(eval_condition(cond)) break;
      }
    }
    else {
      val1 = fetch_data_byte(M.x86.R_ESI);
      val2 = fetch_data_byte_abs(M.x86.seg + R_ES_INDEX, M.x86.R_EDI);
      cmp_byte(val1, val2);
      M.x86.R_ESI += inc;
      M.x86.R_EDI += inc;
    }
  }
  else {
    if(MODE_CODE32) OP_DECODE("a32 ");
    OP_DECODE("cmpsb");

    if(MODE_REP) {
      while(M.x86.R_CX) {
        val1 = fetch_data_byte(M.x86.R_SI);
        val2 = fetch_data_byte_abs(M.x86.seg + R_ES_INDEX, M.x86.R_DI);
        cmp_byte(val1, val2);
        M.x86.R_CX--;
        M.x86.R_SI += inc;
        M.x86.R_DI += inc;
        if(eval_condition(cond)) break;
      }
    }
    else {
      val1 = fetch_data_byte(M.x86.R_SI);
      val2 = fetch_data_byte_abs(M.x86.seg + R_ES_INDEX, M.x86.R_DI);
      cmp_byte(val1, val2);
      M.x86.R_SI += inc;
      M.x86.R_DI += inc;
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xa7
****************************************************************************/
static void x86emuOp_cmps_word(u8 op1)
{
  u32 val1, val2;
  s32 inc;
  unsigned cond;

  if((MODE_ADDR32 && !MODE_CODE32) || (!MODE_ADDR32 && MODE_CODE32)) {	/* xor */
    OP_DECODE("a32 ");
  }

  if(MODE_DATA32) {
    OP_DECODE("cmpsd");
    inc = ACCESS_FLAG(F_DF) ? -4 : 4;
  }
  else {
    OP_DECODE("cmpsw");
    inc = ACCESS_FLAG(F_DF) ? -2 : 2;
  }

  cond = MODE_REPE ? (2 << 1) + 1 /* NZ */ : (2 << 1) /* Z */;

  if(MODE_ADDR32) {
    if(MODE_REP) {
      while(M.x86.R_ECX) {
        if(MODE_DATA32) {
          val1 = fetch_data_long(M.x86.R_ESI);
          val2 = fetch_data_long_abs(M.x86.seg + R_ES_INDEX, M.x86.R_EDI);
          cmp_long(val1, val2);
        }
        else {
          val1 = fetch_data_word(M.x86.R_ESI);
          val2 = fetch_data_word_abs(M.x86.seg + R_ES_INDEX, M.x86.R_EDI);
          cmp_word(val1, val2);
        }
        M.x86.R_ECX--;
        M.x86.R_ESI += inc;
        M.x86.R_EDI += inc;
        if(eval_condition(cond)) break;
      }
    }
    else {
      if(MODE_DATA32) {
        val1 = fetch_data_long(M.x86.R_ESI);
        val2 = fetch_data_long_abs(M.x86.seg + R_ES_INDEX, M.x86.R_EDI);
        cmp_long(val1, val2);
      }
      else {
        val1 = fetch_data_word(M.x86.R_ESI);
        val2 = fetch_data_word_abs(M.x86.seg + R_ES_INDEX, M.x86.R_EDI);
        cmp_word(val1, val2);
      }
      M.x86.R_ESI += inc;
      M.x86.R_EDI += inc;
    }
  }
  else {
    if(MODE_REP) {
      while(M.x86.R_CX) {
        if(MODE_DATA32) {
          val1 = fetch_data_long(M.x86.R_SI);
          val2 = fetch_data_long_abs(M.x86.seg + R_ES_INDEX, M.x86.R_DI);
          cmp_long(val1, val2);
        }
        else {
          val1 = fetch_data_word(M.x86.R_SI);
          val2 = fetch_data_word_abs(M.x86.seg + R_ES_INDEX, M.x86.R_DI);
          cmp_word(val1, val2);
        }
        M.x86.R_CX--;
        M.x86.R_SI += inc;
        M.x86.R_DI += inc;
        if(eval_condition(cond)) break;
      }
    }
    else {
      if(MODE_DATA32) {
        val1 = fetch_data_long(M.x86.R_SI);
        val2 = fetch_data_long_abs(M.x86.seg + R_ES_INDEX, M.x86.R_DI);
        cmp_long(val1, val2);
      }
      else {
        val1 = fetch_data_word(M.x86.R_SI);
        val2 = fetch_data_word_abs(M.x86.seg + R_ES_INDEX, M.x86.R_DI);
        cmp_word(val1, val2);
      }
      M.x86.R_SI += inc;
      M.x86.R_DI += inc;
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xa8
****************************************************************************/
static void x86emuOp_test_AL_IMM(u8 op1)
{
  u8 imm;

  OP_DECODE("test al,");
  imm = fetch_byte();
  DECODE_HEX2(imm);
  test_byte(M.x86.R_AL, imm);
}


/****************************************************************************
REMARKS:
Handles opcode 0xa9
****************************************************************************/
static void x86emuOp_test_AX_IMM(u8 op1)
{
  u32 imm;

  if(MODE_DATA32) {
    OP_DECODE("test eax,");
    imm = fetch_long();
    DECODE_HEX8(imm);
    test_long(M.x86.R_EAX, imm);
  }
  else {
    OP_DECODE("test ax,");
    imm = fetch_word();
    DECODE_HEX4(imm);
    test_word(M.x86.R_AX, imm);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xaa
****************************************************************************/
static void x86emuOp_stos_byte(u8 op1)
{
  s32 inc;
  u32 count;
  u8 val;

  inc = ACCESS_FLAG(F_DF) ? -1 : 1;

  val = M.x86.R_AL;
  count = 1;

  if(MODE_ADDR32) {
    if(!MODE_CODE32) OP_DECODE("a32 ");
    OP_DECODE("stosb");

    if(MODE_REP) {
      count = M.x86.R_ECX;
      M.x86.R_ECX = 0;
    }

    while(count--) {
      store_data_byte_abs(M.x86.seg + R_ES_INDEX, M.x86.R_EDI, val);
      M.x86.R_EDI += inc;
    }
  }
  else {
    if(MODE_CODE32) OP_DECODE("a32 ");
    OP_DECODE("stosb");

    if(MODE_REP) {
      count = M.x86.R_CX;
      M.x86.R_CX = 0;
    }

    while(count--) {
      store_data_byte_abs(M.x86.seg + R_ES_INDEX, M.x86.R_DI, val);
      M.x86.R_DI += inc;
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xab
****************************************************************************/
static void x86emuOp_stos_word(u8 op1)
{
  s32 inc;
  u32 count, val;

  if((MODE_ADDR32 && !MODE_CODE32) || (!MODE_ADDR32 && MODE_CODE32)) {	/* xor */
    OP_DECODE("a32 ");
  }

  if(MODE_DATA32) {
    OP_DECODE("stosd");
    inc = ACCESS_FLAG(F_DF) ? -4 : 4;
  }
  else {
    OP_DECODE("stosw");
    inc = ACCESS_FLAG(F_DF) ? -2 : 2;
  }

  val = M.x86.R_EAX;
  count = 1;

  if(MODE_ADDR32) {
    if(MODE_REP) {
      count = M.x86.R_ECX;
      M.x86.R_ECX = 0;
    }

    while(count--) {
      if(MODE_DATA32) {
        store_data_long_abs(M.x86.seg + R_ES_INDEX, M.x86.R_EDI, val);
      }
      else {
        store_data_word_abs(M.x86.seg + R_ES_INDEX, M.x86.R_EDI, val);
      }
      M.x86.R_EDI += inc;
    }
  }
  else {
    if(MODE_REP) {
      count = M.x86.R_CX;
      M.x86.R_CX = 0;
    }

    while(count--) {
      if(MODE_DATA32) {
        store_data_long_abs(M.x86.seg + R_ES_INDEX, M.x86.R_DI, val);
      }
      else {
        store_data_word_abs(M.x86.seg + R_ES_INDEX, M.x86.R_DI, val);
      }
      M.x86.R_DI += inc;
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xac
****************************************************************************/
static void x86emuOp_lods_byte(u8 op1)
{
  s32 inc;
  u32 count;

  inc = ACCESS_FLAG(F_DF) ? -1 : 1;

  count = 1;

  if(MODE_ADDR32) {
    if(!MODE_CODE32) OP_DECODE("a32 ");
    OP_DECODE("lodsb");

    if(MODE_REP) {
      count = M.x86.R_ECX;
      M.x86.R_ECX = 0;
    }

    while(count--) {
      M.x86.R_AL = fetch_data_byte(M.x86.R_ESI);
      M.x86.R_ESI += inc;
    }
  }
  else {
    if(MODE_CODE32) OP_DECODE("a32 ");
    OP_DECODE("lodsb");

    if(MODE_REP) {
      count = M.x86.R_CX;
      M.x86.R_CX = 0;
    }

    while(count--) {
      M.x86.R_AL = fetch_data_byte(M.x86.R_SI);
      M.x86.R_SI += inc;
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xad
****************************************************************************/
static void x86emuOp_lods_word(u8 op1)
{
  s32 inc;
  u32 count;

  if((MODE_ADDR32 && !MODE_CODE32) || (!MODE_ADDR32 && MODE_CODE32)) {	/* xor */
    OP_DECODE("a32 ");
  }

  if(MODE_DATA32) {
    OP_DECODE("lodsd");
    inc = ACCESS_FLAG(F_DF) ? -4 : 4;
  }
  else {
    OP_DECODE("lodsw");
    inc = ACCESS_FLAG(F_DF) ? -2 : 2;
  }

  count = 1;

  if(MODE_ADDR32) {
    if(MODE_REP) {
      count = M.x86.R_ECX;
      M.x86.R_ECX = 0;
    }

    while(count--) {
      if(MODE_DATA32) {
        M.x86.R_EAX = fetch_data_long(M.x86.R_ESI);
      }
      else {
        M.x86.R_AX = fetch_data_word(M.x86.R_ESI);
      }
      M.x86.R_ESI += inc;
    }
  }
  else {
    if(MODE_REP) {
      count = M.x86.R_CX;
      M.x86.R_CX = 0;
    }

    while(count--) {
      if(MODE_DATA32) {
        M.x86.R_EAX = fetch_data_long(M.x86.R_SI);
      }
      else {
        M.x86.R_AX = fetch_data_word(M.x86.R_SI);
      }
      M.x86.R_SI += inc;
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xae
****************************************************************************/
static void x86emuOp_scas_byte(u8 op1)
{
  s8 val;
  s32 inc;
  unsigned cond;

  inc = ACCESS_FLAG(F_DF) ? -1 : 1;

  cond = MODE_REPE ? (2 << 1) + 1 /* NZ */ : (2 << 1) /* Z */;

  if(MODE_ADDR32) {
    if(!MODE_CODE32) OP_DECODE("a32 ");
    OP_DECODE("scasb");

    if(MODE_REP) {
      while(M.x86.R_ECX) {
        val = fetch_data_byte_abs(M.x86.seg + R_ES_INDEX, M.x86.R_EDI);
        cmp_byte(M.x86.R_AL, val);
        M.x86.R_ECX--;
        M.x86.R_EDI += inc;
        if(eval_condition(cond)) break;
      }
    }
    else {
      val = fetch_data_byte_abs(M.x86.seg + R_ES_INDEX, M.x86.R_EDI);
      cmp_byte(M.x86.R_AL, val);
      M.x86.R_EDI += inc;
    }
  }
  else {
    if(MODE_CODE32) OP_DECODE("a32 ");
    OP_DECODE("scasb");

    if(MODE_REP) {
      while(M.x86.R_CX) {
        val = fetch_data_byte_abs(M.x86.seg + R_ES_INDEX, M.x86.R_DI);
        cmp_byte(M.x86.R_AL, val);
        M.x86.R_CX--;
        M.x86.R_DI += inc;
        if(eval_condition(cond)) break;
      }
    }
    else {
      val = fetch_data_byte_abs(M.x86.seg + R_ES_INDEX, M.x86.R_DI);
      cmp_byte(M.x86.R_AL, val);
      M.x86.R_DI += inc;
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xaf
****************************************************************************/
static void x86emuOp_scas_word(u8 op1)
{
  s32 inc;
  u32 val;
  unsigned cond;

  if((MODE_ADDR32 && !MODE_CODE32) || (!MODE_ADDR32 && MODE_CODE32)) {	/* xor */
    OP_DECODE("a32 ");
  }

  if(MODE_DATA32) {
    OP_DECODE("scasd");
    inc = ACCESS_FLAG(F_DF) ? -4 : 4;
  }
  else {
    OP_DECODE("scasw");
    inc = ACCESS_FLAG(F_DF) ? -2 : 2;
  }

  cond = MODE_REPE ? (2 << 1) + 1 /* NZ */ : (2 << 1) /* Z */;

  if(MODE_ADDR32) {
    if(MODE_REP) {
      while(M.x86.R_ECX) {
        if(MODE_DATA32) {
          val = fetch_data_long_abs(M.x86.seg + R_ES_INDEX, M.x86.R_EDI);
          cmp_long(M.x86.R_EAX, val);
        }
        else {
          val = fetch_data_word_abs(M.x86.seg + R_ES_INDEX, M.x86.R_EDI);
          cmp_word(M.x86.R_AX, val);
        }
        M.x86.R_ECX--;
        M.x86.R_EDI += inc;
        if(eval_condition(cond)) break;
      }
    }
    else {
      if(MODE_DATA32) {
        val = fetch_data_long_abs(M.x86.seg + R_ES_INDEX, M.x86.R_EDI);
        cmp_long(M.x86.R_EAX, val);
      }
      else {
        val = fetch_data_word_abs(M.x86.seg + R_ES_INDEX, M.x86.R_EDI);
        cmp_word(M.x86.R_AX, val);
      }
      M.x86.R_EDI += inc;
    }
  }
  else {
    if(MODE_REP) {
      while(M.x86.R_CX) {
        if(MODE_DATA32) {
          val = fetch_data_long_abs(M.x86.seg + R_ES_INDEX, M.x86.R_DI);
          cmp_long(M.x86.R_EAX, val);
        }
        else {
          val = fetch_data_word_abs(M.x86.seg + R_ES_INDEX, M.x86.R_DI);
          cmp_word(M.x86.R_AX, val);
        }
        M.x86.R_CX--;
        M.x86.R_DI += inc;
        if(eval_condition(cond)) break;
      }
    }
    else {
      if(MODE_DATA32) {
        val = fetch_data_long_abs(M.x86.seg + R_ES_INDEX, M.x86.R_DI);
        cmp_long(M.x86.R_EAX, val);
      }
      else {
        val = fetch_data_word_abs(M.x86.seg + R_ES_INDEX, M.x86.R_DI);
        cmp_word(M.x86.R_AX, val);
      }
      M.x86.R_DI += inc;
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xb0
****************************************************************************/
static void x86emuOp_mov_byte_AL_IMM(u8 op1)
{
  u8 val;

  OP_DECODE("mov al,");
  val = fetch_byte();
  DECODE_HEX2(val);
  M.x86.R_AL = val;
}


/****************************************************************************
REMARKS:
Handles opcode 0xb1
****************************************************************************/
static void x86emuOp_mov_byte_CL_IMM(u8 op1)
{
  u8 val;

  OP_DECODE("mov cl,");
  val = fetch_byte();
  DECODE_HEX2(val);
  M.x86.R_CL = val;
}


/****************************************************************************
REMARKS:
Handles opcode 0xb2
****************************************************************************/
static void x86emuOp_mov_byte_DL_IMM(u8 op1)
{
  u8 val;

  OP_DECODE("mov dl,");
  val = fetch_byte();
  DECODE_HEX2(val);
  M.x86.R_DL = val;
}


/****************************************************************************
REMARKS:
Handles opcode 0xb3
****************************************************************************/
static void x86emuOp_mov_byte_BL_IMM(u8 op1)
{
  u8 val;

  OP_DECODE("mov bl,");
  val = fetch_byte();
  DECODE_HEX2(val);
  M.x86.R_BL = val;
}


/****************************************************************************
REMARKS:
Handles opcode 0xb4
****************************************************************************/
static void x86emuOp_mov_byte_AH_IMM(u8 op1)
{
  u8 val;

  OP_DECODE("mov ah,");
  val = fetch_byte();
  DECODE_HEX2(val);
  M.x86.R_AH = val;
}


/****************************************************************************
REMARKS:
Handles opcode 0xb5
****************************************************************************/
static void x86emuOp_mov_byte_CH_IMM(u8 op1)
{
  u8 val;

  OP_DECODE("mov ch,");
  val = fetch_byte();
  DECODE_HEX2(val);
  M.x86.R_CH = val;
}


/****************************************************************************
REMARKS:
Handles opcode 0xb6
****************************************************************************/
static void x86emuOp_mov_byte_DH_IMM(u8 op1)
{
  u8 val;

  OP_DECODE("mov dh,");
  val = fetch_byte();
  DECODE_HEX2(val);
  M.x86.R_DH = val;
}


/****************************************************************************
REMARKS:
Handles opcode 0xb7
****************************************************************************/
static void x86emuOp_mov_byte_BH_IMM(u8 op1)
{
  u8 val;

  OP_DECODE("mov bh,");
  val = fetch_byte();
  DECODE_HEX2(val);
  M.x86.R_BH = val;
}


/****************************************************************************
REMARKS:
Handles opcode 0xb8
****************************************************************************/
static void x86emuOp_mov_word_AX_IMM(u8 op1)
{
  u32 val;

  if(MODE_DATA32) {
    OP_DECODE("mov eax,");
    val = fetch_long();
    DECODE_HEX8(val);
    M.x86.R_EAX = val;
  }
  else {
    OP_DECODE("mov ax,");
    val = fetch_word();
    DECODE_HEX4(val);
    M.x86.R_AX = val;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xb9
****************************************************************************/
static void x86emuOp_mov_word_CX_IMM(u8 op1)
{
  u32 val;

  if(MODE_DATA32) {
    OP_DECODE("mov ecx,");
    val = fetch_long();
    DECODE_HEX8(val);
    M.x86.R_ECX = val;
  }
  else {
    OP_DECODE("mov cx,");
    val = fetch_word();
    DECODE_HEX4(val);
    M.x86.R_CX = val;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xba
****************************************************************************/
static void x86emuOp_mov_word_DX_IMM(u8 op1)
{
  u32 val;

  if(MODE_DATA32) {
    OP_DECODE("mov edx,");
    val = fetch_long();
    DECODE_HEX8(val);
    M.x86.R_EDX = val;
  }
  else {
    OP_DECODE("mov dx,");
    val = fetch_word();
    DECODE_HEX4(val);
    M.x86.R_DX = val;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xbb
****************************************************************************/
static void x86emuOp_mov_word_BX_IMM(u8 op1)
{
  u32 val;

  if(MODE_DATA32) {
    OP_DECODE("mov ebx,");
    val = fetch_long();
    DECODE_HEX8(val);
    M.x86.R_EBX = val;
  }
  else {
    OP_DECODE("mov bx,");
    val = fetch_word();
    DECODE_HEX4(val);
    M.x86.R_BX = val;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xbc
****************************************************************************/
static void x86emuOp_mov_word_SP_IMM(u8 op1)
{
  u32 val;

  if(MODE_DATA32) {
    OP_DECODE("mov esp,");
    val = fetch_long();
    DECODE_HEX8(val);
    M.x86.R_ESP = val;
  }
  else {
    OP_DECODE("mov sp,");
    val = fetch_word();
    DECODE_HEX4(val);
    M.x86.R_SP = val;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xbd
****************************************************************************/
static void x86emuOp_mov_word_BP_IMM(u8 op1)
{
  u32 val;

  if(MODE_DATA32) {
    OP_DECODE("mov ebp,");
    val = fetch_long();
    DECODE_HEX8(val);
    M.x86.R_EBP = val;
  }
  else {
    OP_DECODE("mov bp,");
    val = fetch_word();
    DECODE_HEX4(val);
    M.x86.R_BP = val;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xbe
****************************************************************************/
static void x86emuOp_mov_word_SI_IMM(u8 op1)
{
  u32 val;

  if(MODE_DATA32) {
    OP_DECODE("mov esi,");
    val = fetch_long();
    DECODE_HEX8(val);
    M.x86.R_ESI = val;
  }
  else {
    OP_DECODE("mov si,");
    val = fetch_word();
    DECODE_HEX4(val);
    M.x86.R_SI = val;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xbf
****************************************************************************/
static void x86emuOp_mov_word_DI_IMM(u8 op1)
{
  u32 val;

  if(MODE_DATA32) {
    OP_DECODE("mov edi,");
    val = fetch_long();
    DECODE_HEX8(val);
    M.x86.R_EDI = val;
  }
  else {
    OP_DECODE("mov di,");
    val = fetch_word();
    DECODE_HEX4(val);
    M.x86.R_DI = val;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xc0
****************************************************************************/
static void x86emuOp_opcC0_byte_RM_MEM(u8 op1)
{
  int mod, rl, rh;
  u8 *reg8, val, imm;
  u32 addr;

  fetch_decode_modrm(&mod, &rh, &rl);

  decode_op_B(rh);

  if(mod == 3) {
    reg8 = decode_rm_byte_register(rl);
    OP_DECODE(",");
    imm = fetch_byte();
    DECODE_HEX2(imm);
    val = (*op_B_byte[rh])(*reg8, imm);
    *reg8 = val;
  }
  else {
    OP_DECODE("byte ");
    addr = decode_rm_address(mod, rl);
    OP_DECODE(",");
    val = fetch_data_byte(addr);
    imm = fetch_byte();
    DECODE_HEX2(imm);
    val = (*op_B_byte[rh])(val, imm);
    store_data_byte(addr, val);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xc1
****************************************************************************/
static void x86emuOp_opcC1_word_RM_MEM(u8 op1)
{
  int mod, rl, rh;
  u8 imm;
  u16 *reg16;
  u32 *reg32, val, addr;

  fetch_decode_modrm(&mod, &rh, &rl);

  decode_op_B(rh);

  if(mod == 3) {
    if(MODE_DATA32) {
      reg32 = decode_rm_long_register(rl);
      OP_DECODE(",");
      imm = fetch_byte();
      DECODE_HEX2(imm);
      val = (*op_B_long[rh])(*reg32, imm);
      *reg32 = val;
    }
    else {
      reg16 = decode_rm_word_register(rl);
      OP_DECODE(",");
      imm = fetch_byte();
      DECODE_HEX2(imm);
      val = (*op_B_word[rh])(*reg16, imm);
      *reg16 = val;
    }
  }
  else {
    if(MODE_DATA32) {
      OP_DECODE("dword ");
      addr = decode_rm_address(mod, rl);
      OP_DECODE(",");
      val = fetch_data_long(addr);
      imm = fetch_byte();
      DECODE_HEX2(imm);
      val = (*op_B_long[rh])(val, imm);
      store_data_long(addr, val);
    }
    else {
      OP_DECODE("word ");
      addr = decode_rm_address(mod, rl);
      OP_DECODE(",");
      val = fetch_data_word(addr);
      imm = fetch_byte();
      DECODE_HEX2(imm);
      val = (*op_B_word[rh])(val, imm);
      store_data_word(addr, val);
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xc2
****************************************************************************/
static void x86emuOp_ret_near_IMM(u8 op1)
{
  u32 imm;

  OP_DECODE("ret ");
  imm = fetch_word();

  DECODE_HEX4(imm);

  if(MODE_DATA32) {
    M.x86.R_EIP = pop_long();
    M.x86.R_ESP += imm;
  }
  else {
    M.x86.R_EIP = pop_word();
    M.x86.R_SP += imm;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xc3
****************************************************************************/
static void x86emuOp_ret_near(u8 op1)
{
  OP_DECODE("ret");

  if(MODE_DATA32) {
    M.x86.R_EIP = pop_long();
  }
  else {
    M.x86.R_EIP = pop_word();
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xc4
****************************************************************************/
static void x86emuOp_les_R_IMM(u8 op1)
{
  int mod, rh, rl;
  u16 *reg16;
  u32 *reg32, addr;

  OP_DECODE("les ");
  fetch_decode_modrm(&mod, &rh, &rl);
  if(mod == 3) {
    INTR_RAISE_UD(&M);
  }
  else {
    if(MODE_DATA32){
      reg32 = decode_rm_long_register(rh);
      OP_DECODE(",");
      addr = decode_rm_address(mod, rl);
      *reg32 = fetch_data_long(addr);
      addr += 4;
    }
    else {
      reg16 = decode_rm_word_register(rh);
      OP_DECODE(",");
      addr = decode_rm_address(mod, rl);
      *reg16 = fetch_data_word(addr);
      addr += 2;
    }
    x86emu_set_seg_register(&M, M.x86.R_ES_SEL, fetch_data_word(addr));
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xc5
****************************************************************************/
static void x86emuOp_lds_R_IMM(u8 op1)
{
  int mod, rh, rl;
  u16 *reg16;
  u32 *reg32, addr;

  OP_DECODE("lds ");
  fetch_decode_modrm(&mod, &rh, &rl);
  if(mod == 3) {
    INTR_RAISE_UD(&M);
  }
  else {
    if(MODE_DATA32){
      reg32 = decode_rm_long_register(rh);
      OP_DECODE(",");
      addr = decode_rm_address(mod, rl);
      *reg32 = fetch_data_long(addr);
      addr += 4;
    }
    else {
      reg16 = decode_rm_word_register(rh);
      OP_DECODE(",");
      addr = decode_rm_address(mod, rl);
      *reg16 = fetch_data_word(addr);
      addr += 2;
    }
    x86emu_set_seg_register(&M, M.x86.R_DS_SEL, fetch_data_word(addr));
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xc6
****************************************************************************/
static void x86emuOp_mov_byte_RM_IMM(u8 op1)
{
  int mod, rl, rh;
  u8 *reg8, imm;
  u32 addr;

  OP_DECODE("mov ");
  fetch_decode_modrm(&mod, &rh, &rl);

  if(rh != 0) {
    INTR_RAISE_UD(&M);
    return;
  }

  if(mod == 3) {
    reg8 = decode_rm_byte_register(rl);
    imm = fetch_byte();
    DECODE_HEX2(imm);
    *reg8 = imm;
  }
  else {
    addr = decode_rm_address(mod, rl);
    OP_DECODE(",");
    imm = fetch_byte();
    DECODE_HEX2(imm);
    store_data_byte(addr, imm);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xc7
****************************************************************************/
static void x86emuOp_mov_word_RM_IMM(u8 op1)
{
  int mod, rl, rh;
  u16 *reg16;
  u32 *reg32, addr, imm;

  OP_DECODE("mov ");
  fetch_decode_modrm(&mod, &rh, &rl);

  if(rh != 0) {
    INTR_RAISE_UD(&M);
  }

  if(mod == 3) {
    if(MODE_DATA32) {
      reg32 = decode_rm_long_register(rl);
      imm = fetch_long();
      DECODE_HEX8(imm);
      *reg32 = imm;
    }
    else {
      reg16 = decode_rm_word_register(rl);
      imm = fetch_word();
      DECODE_HEX4(imm);
      *reg16 = imm;
    }
  }
  else {
    addr = decode_rm_address(mod, rl);
    OP_DECODE(",");

    if(MODE_DATA32) {
      imm = fetch_long();
      DECODE_HEX8(imm);
      store_data_long(addr, imm);
    }
    else {
      imm = fetch_word();
      DECODE_HEX4(imm);
      store_data_word(addr, imm);
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xc8
****************************************************************************/
static void x86emuOp_enter(u8 op1)
{
  u32 local, frame_pointer;
  unsigned i, nesting;

  OP_DECODE("enter ");

  local = fetch_word();
  nesting = fetch_byte();

  DECODE_HEX4(local);
  OP_DECODE(",");
  DECODE_HEX2(nesting);

  nesting &= 0x1f;

  if(MODE_STACK32) {
    push_long(M.x86.R_EBP);
    frame_pointer = M.x86.R_ESP;
  }
  else {
    push_word(M.x86.R_BP);
    frame_pointer = M.x86.R_SP;
  }

  if(nesting > 0) {
    if(MODE_DATA32) {
      for(i = 1; i < nesting; i++) {
        if(MODE_STACK32) {
          M.x86.R_EBP -= 4;
          push_long(fetch_data_long_abs(M.x86.seg + R_SS_INDEX, M.x86.R_EBP));
        }
        else {
          M.x86.R_BP -= 4;
          push_long(fetch_data_long_abs(M.x86.seg + R_SS_INDEX, M.x86.R_BP));
        }
      }
    }
    else {
      for(i = 1; i < nesting; i++) {
        if(MODE_STACK32) {
          M.x86.R_EBP -= 2;
          push_word(fetch_data_word_abs(M.x86.seg + R_SS_INDEX, M.x86.R_EBP));
        }
        else {
          M.x86.R_BP -= 2;
          push_word(fetch_data_word_abs(M.x86.seg + R_SS_INDEX, M.x86.R_BP));
        }
      }
    }

    if(MODE_DATA32) {
      push_long(frame_pointer);
    }
    else {
      push_word(frame_pointer);
    }
  }

  // FIXME: really esp - local (not: ebp - local)???
  if(MODE_STACK32) {
    M.x86.R_EBP = frame_pointer;
    M.x86.R_ESP = M.x86.R_ESP - local;
  }
  else {
    M.x86.R_BP = frame_pointer;
    M.x86.R_SP = M.x86.R_SP - local;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xc9
****************************************************************************/
static void x86emuOp_leave(u8 op1)
{
  OP_DECODE("leave");

  if(MODE_STACK32) {
    M.x86.R_ESP = M.x86.R_EBP;
  }
  else {
    M.x86.R_SP = M.x86.R_BP;
  }

  if(MODE_DATA32) {
    M.x86.R_EBP = pop_long();
  }
  else {
    M.x86.R_BP = pop_word();
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xca
****************************************************************************/
static void x86emuOp_ret_far_IMM(u8 op1)
{
  u16 cs;
  u32 imm, eip;

  OP_DECODE("retf ");
  imm = fetch_word();

  DECODE_HEX4(imm);

  if(MODE_DATA32) {
    eip = pop_long();
    cs = pop_long();
    M.x86.R_ESP += imm;
  }
  else {
    eip = pop_word();
    cs = pop_word();
    M.x86.R_SP += imm;
  }

  x86emu_set_seg_register(&M, M.x86.R_CS_SEL, cs);
  M.x86.R_EIP = eip;
}


/****************************************************************************
REMARKS:
Handles opcode 0xcb
****************************************************************************/
static void x86emuOp_ret_far(u8 op1)
{
  u16 cs;
  u32 eip;

  OP_DECODE("retf");

  if(MODE_DATA32) {
    eip = pop_long();
    cs = pop_long();
  }
  else {
    eip = pop_word();
    cs = pop_word();
  }

  x86emu_set_seg_register(&M, M.x86.R_CS_SEL, cs);
  M.x86.R_EIP = eip;
}


/****************************************************************************
REMARKS:
Handles opcode 0xcc
****************************************************************************/
static void x86emuOp_int3(u8 op1)
{
  OP_DECODE("int 3");

  INTR_RAISE_SOFT(&M, 3);
}


/****************************************************************************
REMARKS:
Handles opcode 0xcd
****************************************************************************/
static void x86emuOp_int_IMM(u8 op1)
{
  u8 nr;

  OP_DECODE("int ");
  nr = fetch_byte();
  DECODE_HEX2(nr);

  INTR_RAISE_SOFT(&M, nr);
}


/****************************************************************************
REMARKS:
Handles opcode 0xce
****************************************************************************/
static void x86emuOp_into(u8 op1)
{
  OP_DECODE("into");

  if(ACCESS_FLAG(F_OF)) INTR_RAISE_SOFT(&M, 4);
}


/****************************************************************************
REMARKS:
Handles opcode 0xcf
****************************************************************************/
static void x86emuOp_iret(u8 op1)
{
  u16 cs;
  u32 eip;

  OP_DECODE("iret");

  if(MODE_DATA32) {   
    eip = pop_long();
    cs = pop_long();
    M.x86.R_EFLG = pop_long() | F_ALWAYS_ON;
  }
  else {
    eip = pop_word();
    cs = pop_word();
    M.x86.R_FLG = pop_word() | F_ALWAYS_ON;
  }
 
  x86emu_set_seg_register(&M, M.x86.R_CS_SEL, cs);
  M.x86.R_EIP = eip;
}


/****************************************************************************
REMARKS:
Handles opcode 0xd0
****************************************************************************/
static void x86emuOp_opcD0_byte_RM_1(u8 op1)
{
  int mod, rl, rh;
  u8 *reg8, val;
  u32 addr;

  fetch_decode_modrm(&mod, &rh, &rl);

  decode_op_B(rh);

  if(mod == 3) {
    reg8 = decode_rm_byte_register(rl);
    OP_DECODE(",1");
    val = (*op_B_byte[rh])(*reg8, 1);
    *reg8 = val;
  }
  else {
    OP_DECODE("byte ");
    addr = decode_rm_address(mod, rl);
    OP_DECODE(",1");
    val = fetch_data_byte(addr);
    val = (*op_B_byte[rh])(val, 1);
    store_data_byte(addr, val);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xd1
****************************************************************************/
static void x86emuOp_opcD1_word_RM_1(u8 op1)
{
  int mod, rl, rh;
  u16 *reg16;
  u32 *reg32, val, addr;

  fetch_decode_modrm(&mod, &rh, &rl);

  decode_op_B(rh);

  if(mod == 3) {
    if(MODE_DATA32) {
      reg32 = decode_rm_long_register(rl);
      OP_DECODE(",1");
      val = (*op_B_long[rh])(*reg32, 1);
      *reg32 = val;
    }
    else {
      reg16 = decode_rm_word_register(rl);
      OP_DECODE(",1");
      val = (*op_B_word[rh])(*reg16, 1);
      *reg16 = val;
    }
  }
  else {
    if(MODE_DATA32) {
      OP_DECODE("dword ");
      addr = decode_rm_address(mod, rl);
      OP_DECODE(",1");
      val = fetch_data_long(addr);
      val = (*op_B_long[rh])(val, 1);
      store_data_long(addr, val);
    }
    else {
      OP_DECODE("word ");
      addr = decode_rm_address(mod, rl);
      OP_DECODE(",1");
      val = fetch_data_word(addr);
      val = (*op_B_word[rh])(val, 1);
      store_data_word(addr, val);
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xd2
****************************************************************************/
static void x86emuOp_opcD2_byte_RM_CL(u8 op1)
{
  int mod, rl, rh;
  u8 *reg8, val, imm;
  u32 addr;

  fetch_decode_modrm(&mod, &rh, &rl);

  decode_op_B(rh);

  imm = M.x86.R_CL;

  if(mod == 3) {
    reg8 = decode_rm_byte_register(rl);
    OP_DECODE(",cl");
    val = (*op_B_byte[rh])(*reg8, imm);
    *reg8 = val;
  }
  else {
    OP_DECODE("byte ");
    addr = decode_rm_address(mod, rl);
    OP_DECODE(",cl");
    val = fetch_data_byte(addr);
    val = (*op_B_byte[rh])(val, imm);
    store_data_byte(addr, val);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xd3
****************************************************************************/
static void x86emuOp_opcD3_word_RM_CL(u8 op1)
{
  int mod, rl, rh;
  u8 imm;
  u16 *reg16;
  u32 *reg32, val, addr;

  fetch_decode_modrm(&mod, &rh, &rl);

  decode_op_B(rh);

  imm = M.x86.R_CL;

  if(mod == 3) {
    if(MODE_DATA32) {
      reg32 = decode_rm_long_register(rl);
      OP_DECODE(",cl");
      val = (*op_B_long[rh])(*reg32, imm);
      *reg32 = val;
    }
    else {
      reg16 = decode_rm_word_register(rl);
      OP_DECODE(",cl");
      val = (*op_B_word[rh])(*reg16, imm);
      *reg16 = val;
    }
  }
  else {
    if(MODE_DATA32) {
      OP_DECODE("dword ");
      addr = decode_rm_address(mod, rl);
      OP_DECODE(",cl");
      val = fetch_data_long(addr);
      val = (*op_B_long[rh])(val, imm);
      store_data_long(addr, val);
    }
    else {
      OP_DECODE("word ");
      addr = decode_rm_address(mod, rl);
      OP_DECODE(",cl");
      val = fetch_data_word(addr);
      val = (*op_B_word[rh])(val, imm);
      store_data_word(addr, val);
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xd4
****************************************************************************/
static void x86emuOp_aam(u8 op1)
{
  u8 base;

  OP_DECODE("aam");

  base = fetch_byte();
  if(base == 0) INTR_RAISE_DIV0(&M);

  M.x86.R_AX = aam_word(M.x86.R_AL, base);
}


/****************************************************************************
REMARKS:
Handles opcode 0xd5
****************************************************************************/
static void x86emuOp_aad(u8 op1)
{
  u8 base;

  OP_DECODE("aad");
  base = fetch_byte();

  M.x86.R_AX = aad_word(M.x86.R_AX, base);
}


/****************************************************************************
REMARKS:
Handles opcode 0xd6
****************************************************************************/
static void x86emuOp_setalc(u8 op1)
{
  OP_DECODE("setalc");

  M.x86.R_AL = ACCESS_FLAG(F_CF) ? 0xff : 0;
}


/****************************************************************************
REMARKS:
Handles opcode 0xd7
****************************************************************************/
static void x86emuOp_xlat(u8 op1)
{
  u32 addr;

  OP_DECODE("xlat");

  if(MODE_DATA32) {
    addr = M.x86.R_EBX + M.x86.R_AL;
  }
  else {
    addr = (M.x86.R_BX + M.x86.R_AL) & 0xffff;
  }

  M.x86.R_AL = fetch_data_byte(addr);
}


/* instuctions  D8 .. DF are in i87_ops.c */


/****************************************************************************
REMARKS:
Handles opcode 0xe0
****************************************************************************/
static void x86emuOp_loopne(u8 op1)
{
  s32 ofs;
  u32 eip;

  OP_DECODE("loopnz ");
  ofs = (s8) fetch_byte();
  eip = M.x86.R_EIP + ofs;
  DECODE_HEX_ADDR(eip);

  if(MODE_DATA32) {
    if(--M.x86.R_ECX && !ACCESS_FLAG(F_ZF)) M.x86.R_EIP = eip;
  }
  else {
    eip &= 0xffff;      // FIXME: is not correct
    if(--M.x86.R_CX && !ACCESS_FLAG(F_ZF)) M.x86.R_EIP = eip;
  }

}


/****************************************************************************
REMARKS:
Handles opcode 0xe1
****************************************************************************/
static void x86emuOp_loope(u8 op1)
{
  s32 ofs;
  u32 eip;

  OP_DECODE("loopz ");
  ofs = (s8) fetch_byte();
  eip = M.x86.R_EIP + ofs;
  DECODE_HEX_ADDR(eip);

  if(MODE_DATA32) {
    if(--M.x86.R_ECX && ACCESS_FLAG(F_ZF)) M.x86.R_EIP = eip;
  }
  else {
    eip &= 0xffff;      // FIXME: is not correct
    if(--M.x86.R_CX && ACCESS_FLAG(F_ZF)) M.x86.R_EIP = eip;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xe2
****************************************************************************/
static void x86emuOp_loop(u8 op1)
{
  s32 ofs;
  u32 eip;

  OP_DECODE("loop ");
  ofs = (s8) fetch_byte();
  eip = M.x86.R_EIP + ofs;
  DECODE_HEX_ADDR(eip);

  if(MODE_DATA32) {
    if(--M.x86.R_ECX) M.x86.R_EIP = eip;
  }
  else {
    eip &= 0xffff;      // FIXME: is not correct
    if(--M.x86.R_CX) M.x86.R_EIP = eip;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xe3
****************************************************************************/
static void x86emuOp_jcxz(u8 op1)
{
  s32 ofs;
  u32 eip;

  ofs = (s8) fetch_byte();
  eip = M.x86.R_EIP + ofs;

  if(MODE_DATA32) {
    OP_DECODE("jecxz ");
    DECODE_HEX_ADDR(eip);
    if(M.x86.R_ECX == 0) M.x86.R_EIP = eip;
  }
  else {
    OP_DECODE("jcxz ");
    eip &= 0xffff;      // FIXME: is not correct
    DECODE_HEX_ADDR(eip);
    if(M.x86.R_CX == 0) M.x86.R_EIP = eip;
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xe4
****************************************************************************/
static void x86emuOp_in_byte_AL_IMM(u8 op1)
{
  u8 port;

  OP_DECODE("in al,");
  port = fetch_byte();
  DECODE_HEX2(port);

  M.x86.R_AL = fetch_io_byte(port);
}


/****************************************************************************
REMARKS:
Handles opcode 0xe5
****************************************************************************/
static void x86emuOp_in_word_AX_IMM(u8 op1)
{
  u8 port;

  OP_DECODE("in ");
  port = fetch_byte();

  if(MODE_DATA32) {
    OP_DECODE("eax,");
    M.x86.R_EAX = fetch_io_long(port);
  } else {
    OP_DECODE("ax,");
    M.x86.R_AX = fetch_io_word(port);
  }

  DECODE_HEX2(port);
}


/****************************************************************************
REMARKS:
Handles opcode 0xe6
****************************************************************************/
static void x86emuOp_out_byte_IMM_AL(u8 op1)
{
  u8 port;

  OP_DECODE("out ");
  port = fetch_byte();
  DECODE_HEX2(port);
  OP_DECODE(",al");

  store_io_byte(port, M.x86.R_AL);
}


/****************************************************************************
REMARKS:
Handles opcode 0xe7
****************************************************************************/
static void x86emuOp_out_word_IMM_AX(u8 op1)
{
  u8 port;

  OP_DECODE("out ");
  port = fetch_byte();
  DECODE_HEX2(port);

  if(MODE_DATA32) {
    OP_DECODE(",eax");
    store_io_long(port, M.x86.R_EAX);
  }
  else {
    OP_DECODE(",ax");
    store_io_word(port, M.x86.R_AX);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xe8
****************************************************************************/
static void x86emuOp_call_near_IMM(u8 op1)
{
  s32 ofs;
  u32 eip;

  OP_DECODE("call ");
  if(MODE_DATA32) {
    ofs = fetch_long();
  }
  else {
    ofs = (s16) fetch_word();
  }

  eip = M.x86.R_EIP + ofs;

  if(MODE_DATA32) {
    DECODE_HEX_ADDR(eip);

    push_long(M.x86.R_EIP);
  }
  else {
    eip &= 0xffff;	// FIXME: is not correct
    DECODE_HEX_ADDR(eip);

    push_word(M.x86.R_IP);
  }

  M.x86.R_EIP = eip;
}


/****************************************************************************
REMARKS:
Handles opcode 0xe9
****************************************************************************/
static void x86emuOp_jump_near_IMM(u8 op1)
{
  s32 ofs;
  u32 eip;

  OP_DECODE("jmp ");

  if(MODE_DATA32) {
    ofs = fetch_long();
  }
  else {
    ofs = (s16) fetch_word();
  }

  eip = M.x86.R_EIP + ofs;

  if(!MODE_DATA32) eip &= 0xffff;

  DECODE_HEX_ADDR(eip);

  M.x86.R_EIP = eip;
}


/****************************************************************************
REMARKS:
Handles opcode 0xea
****************************************************************************/
static void x86emuOp_jump_far_IMM(u8 op1)
{
  u16 cs;
  u32 eip;

  OP_DECODE("jmp far ");
  eip = MODE_DATA32 ? fetch_long() : fetch_word();
  cs = fetch_word();

  x86emu_set_seg_register(&M, M.x86.R_CS_SEL, cs);
  M.x86.R_EIP = eip;

  DECODE_HEX4(cs);
  OP_DECODE(":");
  if(MODE_DATA32) {
    DECODE_HEX8(eip);
  }
  else {
    DECODE_HEX4(eip);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xeb
****************************************************************************/
static void x86emuOp_jump_byte_IMM(u8 op1)
{
  s32 ofs;
  u32 eip;

  OP_DECODE("jmp ");
  ofs = (s8) fetch_byte();

  eip = M.x86.R_EIP + ofs;

  if(MODE_DATA32) {
    DECODE_HEX_ADDR(eip);
  }
  else {
    eip &= 0xffff;	// FIXME: is not correct
    DECODE_HEX_ADDR(eip);
  }

  // we had a prefix: special debug instruction
  if((M.log.trace & X86EMU_TRACE_DEBUG) && M.x86.R_EIP - M.x86.saved_eip == 3 && ofs >= 1) {
    M.x86.debug_start = M.x86.R_CS_BASE + M.x86.R_EIP;
    M.x86.debug_len = ofs;
  }

  M.x86.R_EIP = eip;
}


/****************************************************************************
REMARKS:
Handles opcode 0xec
****************************************************************************/
static void x86emuOp_in_byte_AL_DX(u8 op1)
{
  OP_DECODE("in al,dx");
  M.x86.R_AL = fetch_io_byte(M.x86.R_DX);
}


/****************************************************************************
REMARKS:
Handles opcode 0xed
****************************************************************************/
static void x86emuOp_in_word_AX_DX(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("in eax,dx");
    M.x86.R_EAX = fetch_io_long(M.x86.R_DX);
  }
  else {
    OP_DECODE("in ax,dx");
    M.x86.R_AX = fetch_io_word(M.x86.R_DX);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xee
****************************************************************************/
static void x86emuOp_out_byte_DX_AL(u8 op1)
{
  OP_DECODE("out dx,al");
  store_io_byte(M.x86.R_DX, M.x86.R_AL);
}


/****************************************************************************
REMARKS:
Handles opcode 0xef
****************************************************************************/
static void x86emuOp_out_word_DX_AX(u8 op1)
{
  if(MODE_DATA32) {
    OP_DECODE("out dx,eax");
    store_io_long(M.x86.R_DX, M.x86.R_EAX);
  }
  else {
    OP_DECODE("out dx,ax");
    store_io_word(M.x86.R_DX, M.x86.R_AX);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xf4
****************************************************************************/
static void x86emuOp_hlt(u8 op1)
{
  OP_DECODE("hlt");
  x86emu_stop(&M);
}


/****************************************************************************
REMARKS:
Handles opcode 0xf5
****************************************************************************/
static void x86emuOp_cmc(u8 op1)
{
  /* complement the carry flag. */
  OP_DECODE("cmc");
  TOGGLE_FLAG(F_CF);
}


/****************************************************************************
REMARKS:
Handles opcode 0xf6
****************************************************************************/
static void x86emuOp_opcF6_byte_RM(u8 op1)
{
  int mod, rl, rh;
  u8 *reg8, val, imm;
  u32 addr;

  fetch_decode_modrm(&mod, &rh, &rl);

  if(mod == 3) {
    switch(rh) {
      case 0:
      case 1:	/* test imm */
        OP_DECODE("test ");
        reg8 = decode_rm_byte_register(rl);
        OP_DECODE(",");
        imm = fetch_byte();
        DECODE_HEX2(imm);
        test_byte(*reg8, imm);
        break;

      case 2:	/* not */
        OP_DECODE("not ");
        reg8 = decode_rm_byte_register(rl);
        *reg8 = not_byte(*reg8);
        break;

      case 3:	/* neg */
        OP_DECODE("neg ");
        reg8 = decode_rm_byte_register(rl);
        *reg8 = neg_byte(*reg8);
        break;

      case 4:	/* mul al, */
        OP_DECODE("mul ");
        reg8 = decode_rm_byte_register(rl);
        mul_byte(*reg8);
        break;

      case 5:	/* imul al, */
        OP_DECODE("imul ");
        reg8 = decode_rm_byte_register(rl);
        imul_byte(*reg8);
        break;

      case 6:	/* div ax, */
        OP_DECODE("div ");
        reg8 = decode_rm_byte_register(rl);
        div_byte(*reg8);
        break;

      case 7:	/* idiv ax, */
        OP_DECODE("idiv ");
        reg8 = decode_rm_byte_register(rl);
        idiv_byte(*reg8);
        break;
    }
  }
  else {
    switch(rh) {
      case 0:
      case 1:	/* test imm */
        OP_DECODE("test byte ");
        addr = decode_rm_address(mod, rl);
        OP_DECODE(",");
        imm = fetch_byte();
        DECODE_HEX2(imm);
        val = fetch_data_byte(addr);
        test_byte(val, imm);
        break;

      case 2:	/* not */
        OP_DECODE("not byte ");
        addr = decode_rm_address(mod, rl);
        val = fetch_data_byte(addr);
        val = not_byte(val);
        store_data_byte(addr, val);
        break;

      case 3:	/* neg */
        OP_DECODE("neg byte ");
        addr = decode_rm_address(mod, rl);
        val = fetch_data_byte(addr);
        val = neg_byte(val);
        store_data_byte(addr, val);
        break;

      case 4:	/* mul al, */
        OP_DECODE("mul byte ");
        addr = decode_rm_address(mod, rl);
        val = fetch_data_byte(addr);
        mul_byte(val);
        break;

      case 5:	/* imul al, */
        OP_DECODE("imul byte ");
        addr = decode_rm_address(mod, rl);
        val = fetch_data_byte(addr);
        imul_byte(val);
        break;

      case 6:	/* div ax, */
        OP_DECODE("div byte ");
        addr = decode_rm_address(mod, rl);
        val = fetch_data_byte(addr);
        div_byte(val);
        break;

      case 7:	/* idiv ax, */
        OP_DECODE("idiv byte ");
        addr = decode_rm_address(mod, rl);
        val = fetch_data_byte(addr);
        idiv_byte(val);
        break;
    }
  }
}

/****************************************************************************
REMARKS:
Handles opcode 0xf7
****************************************************************************/
static void x86emuOp_opcF7_word_RM(u8 op1)
{
  int mod, rl, rh;
  u16 *reg16;
  u32 *reg32, addr, imm, val;

  fetch_decode_modrm(&mod, &rh, &rl);

  if(mod == 3) {
    switch(rh) {
      case 0:
      case 1:	/* test imm */
        OP_DECODE("test ");

        if(MODE_DATA32) {
          reg32 = decode_rm_long_register(rl);
          OP_DECODE(",");
          imm = fetch_long();
          DECODE_HEX8(imm);
          test_long(*reg32, imm);
        }
        else {
          reg16 = decode_rm_word_register(rl);
          OP_DECODE(",");
          imm = fetch_word();
          DECODE_HEX4(imm);
          test_word(*reg16, imm);
        }
        break;

      case 2:	/* not */
        OP_DECODE("not ");

        if(MODE_DATA32) {
          reg32 = decode_rm_long_register(rl);
          *reg32 = not_long(*reg32);
        }
        else {
          reg16 = decode_rm_word_register(rl);
          *reg16 = not_word(*reg16);
        }
        break;

      case 3:	/* neg */
        OP_DECODE("neg ");

        if(MODE_DATA32) {
          reg32 = decode_rm_long_register(rl);
          *reg32 = neg_long(*reg32);
        }
        else {
          reg16 = decode_rm_word_register(rl);
          *reg16 = neg_word(*reg16);
        }
        break;

      case 4:	/* mul ax, */
        OP_DECODE("mul ");

        if(MODE_DATA32) {
          reg32 = decode_rm_long_register(rl);
          mul_long(*reg32);
        }
        else {
          reg16 = decode_rm_word_register(rl);
          mul_word(*reg16);
        }
        break;

      case 5:	/* imul ax, */
        OP_DECODE("imul ");

        if(MODE_DATA32) {
          reg32 = decode_rm_long_register(rl);
          imul_long(*reg32);
        }
        else {
          reg16 = decode_rm_word_register(rl);
          imul_word(*reg16);
        }
        break;

      case 6:	/* div dx:ax, */
        OP_DECODE("div ");

        if(MODE_DATA32) {
          reg32 = decode_rm_long_register(rl);
          div_long(*reg32);
        }
        else {
          reg16 = decode_rm_word_register(rl);
          div_word(*reg16);
        }
        break;

      case 7:	/* idiv dx:ax, */
        OP_DECODE("idiv ");

        if(MODE_DATA32) {
          reg32 = decode_rm_long_register(rl);
          idiv_long(*reg32);
        }
        else {
          reg16 = decode_rm_word_register(rl);
          idiv_word(*reg16);
        }
        break;
    }
  }
  else {
    switch(rh) {
      case 0:
      case 1:	/* test imm */
        if(MODE_DATA32) {
          OP_DECODE("test dword ");
          addr = decode_rm_address(mod, rl);
          OP_DECODE(",");
          imm = fetch_long();
          DECODE_HEX8(imm);
          val = fetch_data_long(addr);
          test_long(val, imm);
        }
        else {
          OP_DECODE("test word ");
          addr = decode_rm_address(mod, rl);
          OP_DECODE(",");
          imm = fetch_word();
          DECODE_HEX4(imm);
          val = fetch_data_word(addr);
          test_word(val, imm);
        }
        break;

      case 2:	/* not */
        if(MODE_DATA32) {
          OP_DECODE("not dword ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_long(addr);
          val = not_long(val);
          store_data_long(addr, val);
        }
        else {
          OP_DECODE("not word ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_word(addr);
          val = not_word(val);
          store_data_word(addr, val);
        }
        break;

      case 3:	/* neg */
        if(MODE_DATA32) {
          OP_DECODE("neg dword ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_long(addr);
          val = neg_long(val);
          store_data_long(addr, val);
        }
        else {
          OP_DECODE("neg word ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_word(addr);
          val = neg_word(val);
          store_data_word(addr, val);
        }
        break;

      case 4:	/* mul ax, */
        if(MODE_DATA32) {
          OP_DECODE("mul dword ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_long(addr);
          mul_long(val);
        }
        else {
          OP_DECODE("mul word ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_word(addr);
          mul_word(val);
        }
        break;

      case 5:	/* imul ax, */
        if(MODE_DATA32) {
          OP_DECODE("imul dword ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_long(addr);
          imul_long(val);
        }
        else {
          OP_DECODE("imul word ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_word(addr);
          imul_word(val);
        }
        break;

      case 6:	/* div dx:ax, */
        if(MODE_DATA32) {
          OP_DECODE("div dword ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_long(addr);
          div_long(val);
        }
        else {
          OP_DECODE("div word ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_word(addr);
          div_word(val);
        }
        break;

      case 7:	/* idiv dx:ax, */
        if(MODE_DATA32) {
          OP_DECODE("idiv dword ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_long(addr);
          idiv_long(val);
        }
        else {
          OP_DECODE("idiv word ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_word(addr);
          idiv_word(val);
        }
        break;
    }
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xf8
****************************************************************************/
static void x86emuOp_clc(u8 op1)
{
  /* clear carry flag. */
  OP_DECODE("clc");
  CLEAR_FLAG(F_CF);
}


/****************************************************************************
REMARKS:
Handles opcode 0xf9
****************************************************************************/
static void x86emuOp_stc(u8 op1)
{
  /* set carry flag. */
  OP_DECODE("stc");
  SET_FLAG(F_CF);
}


/****************************************************************************
REMARKS:
Handles opcode 0xfa
****************************************************************************/
static void x86emuOp_cli(u8 op1)
{
  /* clear interrupts. */
  OP_DECODE("cli");
  CLEAR_FLAG(F_IF);
}


/****************************************************************************
REMARKS:
Handles opcode 0xfb
****************************************************************************/
static void x86emuOp_sti(u8 op1)
{
  /* enable interrupts. */
  OP_DECODE("sti");
  SET_FLAG(F_IF);
}


/****************************************************************************
REMARKS:
Handles opcode 0xfc
****************************************************************************/
static void x86emuOp_cld(u8 op1)
{
  /* direction = increment */
  OP_DECODE("cld");
  CLEAR_FLAG(F_DF);
}


/****************************************************************************
REMARKS:
Handles opcode 0xfd
****************************************************************************/
static void x86emuOp_std(u8 op1)
{
  /* direction = decrement */
  OP_DECODE("std");
  SET_FLAG(F_DF);
}


/****************************************************************************
REMARKS:
Handles opcode 0xfe
****************************************************************************/
static void x86emuOp_opcFE_byte_RM(u8 op1)
{
  int mod, rh, rl;
  u8 *reg8, val;
  u32 addr;

  fetch_decode_modrm(&mod, &rh, &rl);

  switch(rh) {
    case 0:
      OP_DECODE("inc ");
      break;

    case 1:
      OP_DECODE("dec ");
      break;

    default:
      INTR_RAISE_UD(&M);
      return;
  }

  if(mod == 3) {
    reg8 = decode_rm_byte_register(rl);
    *reg8 = rh == 0 ? inc_byte(*reg8) : dec_byte(*reg8);
  }
  else {
    OP_DECODE("byte ");
    addr = decode_rm_address(mod, rl);
    val = fetch_data_byte(addr);
    val = rh == 0 ? inc_byte(val) : dec_byte(val);
    store_data_byte(addr, val);
  }
}


/****************************************************************************
REMARKS:
Handles opcode 0xff
****************************************************************************/
static void x86emuOp_opcFF_word_RM(u8 op1)
{
  int mod, rh, rl;
  u16 *reg16, cs;
  u32 *reg32, addr, val;

  fetch_decode_modrm(&mod, &rh, &rl);

  if(mod == 3) {
    switch(rh) {
      case 0:	/* inc */
        OP_DECODE("inc ");

        if(MODE_DATA32) {
          reg32 = decode_rm_long_register(rl);
          *reg32 = inc_long(*reg32);
        }
        else {
          reg16 = decode_rm_word_register(rl);
          *reg16 = inc_word(*reg16);
        }
        break;

      case 1:	/* dec */
        OP_DECODE("dec ");

        if(MODE_DATA32) {
          reg32 = decode_rm_long_register(rl);
          *reg32 = dec_long(*reg32);
        }
        else {
          reg16 = decode_rm_word_register(rl);
          *reg16 = dec_word(*reg16);
        }
        break;

      case 2:	/* call */
        OP_DECODE("call ");

      	if(MODE_DATA32) {
          reg32 = decode_rm_long_register(rl);
          push_long(M.x86.R_EIP);
          M.x86.R_EIP = *reg32;
      	}
      	else {
          reg16 = decode_rm_word_register(rl);
          push_word(M.x86.R_IP);
          M.x86.R_EIP = *reg16;
        }
        break;

      case 4:	/* jmp */
        OP_DECODE("jmp ");

        if(MODE_DATA32) {
          reg32 = decode_rm_long_register(rl);
          M.x86.R_EIP = *reg32;
        }
        else {
          reg16 = decode_rm_word_register(rl);
          M.x86.R_EIP = *reg16;
        }
        break;

      case 6:
        OP_DECODE("push ");

        if(MODE_DATA32) {
          reg32 = decode_rm_long_register(rl);
          push_long(*reg32);
        }
        else {
          reg16 = decode_rm_word_register(rl);
          push_word(*reg16);
        }
        break;

      default:
        INTR_RAISE_UD(&M);
        break;
    }
  }
  else {
    switch(rh) {
      case 0:	/* inc */
        OP_DECODE("inc ");

        if(MODE_DATA32) {
          OP_DECODE("dword ");

          addr = decode_rm_address(mod, rl);
          val = fetch_data_long(addr);
          val = inc_long(val);
          store_data_long(addr, val);
        }
        else {
          OP_DECODE("word ");

          addr = decode_rm_address(mod, rl);
          val = fetch_data_word(addr);
          val = inc_word(val);
          store_data_word(addr, val);
        }
        break;

      case 1:	/* dec */
        OP_DECODE("dec ");

        if(MODE_DATA32) {
          OP_DECODE("dword ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_long(addr);
          val = dec_long(val);
          store_data_long(addr, val);
        }
        else {
          OP_DECODE("word ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_word(addr);
          val = dec_word(val);
          store_data_word(addr, val);
        }
        break;

      case 2:	/* call */
        OP_DECODE("call ");

        if(MODE_DATA32) {
          OP_DECODE("dword ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_long(addr);
          push_long(M.x86.R_EIP);
        }
        else {
          OP_DECODE("word ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_word(addr);
          push_word(M.x86.R_IP);
        }
        M.x86.R_EIP = val;
        break;

      case 3:	/* call far */
        OP_DECODE("call far ");

        if(MODE_DATA32) {
          if(!MODE_CODE32) OP_DECODE("word ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_long(addr);
          cs = fetch_data_word(addr + 4);

          push_long(M.x86.R_CS);
          push_long(M.x86.R_EIP);
        }
        else {
          if(MODE_CODE32) OP_DECODE("dword ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_word(addr);
          cs = fetch_data_word(addr + 2);

          push_word(M.x86.R_CS);
          push_word(M.x86.R_IP);
        }

        x86emu_set_seg_register(&M, M.x86.R_CS_SEL, cs);
        M.x86.R_EIP = val;
        break;

      case 4:	/* jmp */
        OP_DECODE("jmp ");

        if(MODE_DATA32) {
          OP_DECODE("dword ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_long(addr);
        }
        else {
          OP_DECODE("word ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_word(addr);
        }
        M.x86.R_EIP = val;
        break;

      case 5:	/* jmp far */
        OP_DECODE("jmp far ");

        if(MODE_DATA32) {
          if(!MODE_CODE32) OP_DECODE("word ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_long(addr);
          cs = fetch_data_word(addr + 4);
        }
        else {
          if(MODE_CODE32) OP_DECODE("dword ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_word(addr);
          cs = fetch_data_word(addr + 2);
        }
        x86emu_set_seg_register(&M, M.x86.R_CS_SEL, cs);
        M.x86.R_EIP = val;
        break;

      case 6:	/*  push */
        OP_DECODE("push ");

        if(MODE_DATA32) {
          OP_DECODE("dword ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_long(addr);
          push_long(val);
        }
        else {
          OP_DECODE("word ");
          addr = decode_rm_address(mod, rl);
          val = fetch_data_word(addr);
          push_word(val);
        }
        break;

      case 7:
        INTR_RAISE_UD(&M);
        break;
    }
  }
}


/***************************************************************************
 * Single byte operation code table:
 **************************************************************************/
void (*x86emu_optab[256])(u8) =
{
  /*  0x00 */ x86emuOp_op_A_byte_RM_R,
  /*  0x01 */ x86emuOp_op_A_word_RM_R,
  /*  0x02 */ x86emuOp_op_A_byte_R_RM,
  /*  0x03 */ x86emuOp_op_A_word_R_RM,
  /*  0x04 */ x86emuOp_op_A_byte_AL_IMM,
  /*  0x05 */ x86emuOp_op_A_word_AX_IMM,
  /*  0x06 */ x86emuOp_push_ES,
  /*  0x07 */ x86emuOp_pop_ES,

  /*  0x08 */ x86emuOp_op_A_byte_RM_R,
  /*  0x09 */ x86emuOp_op_A_word_RM_R,
  /*  0x0a */ x86emuOp_op_A_byte_R_RM,
  /*  0x0b */ x86emuOp_op_A_word_R_RM,
  /*  0x0c */ x86emuOp_op_A_byte_AL_IMM,
  /*  0x0d */ x86emuOp_op_A_word_AX_IMM,
  /*  0x0e */ x86emuOp_push_CS,
  /*  0x0f */ x86emuOp_two_byte,

  /*  0x10 */ x86emuOp_op_A_byte_RM_R,
  /*  0x11 */ x86emuOp_op_A_word_RM_R,
  /*  0x12 */ x86emuOp_op_A_byte_R_RM,
  /*  0x13 */ x86emuOp_op_A_word_R_RM,
  /*  0x14 */ x86emuOp_op_A_byte_AL_IMM,
  /*  0x15 */ x86emuOp_op_A_word_AX_IMM,
  /*  0x16 */ x86emuOp_push_SS,
  /*  0x17 */ x86emuOp_pop_SS,

  /*  0x18 */ x86emuOp_op_A_byte_RM_R,
  /*  0x19 */ x86emuOp_op_A_word_RM_R,
  /*  0x1a */ x86emuOp_op_A_byte_R_RM,
  /*  0x1b */ x86emuOp_op_A_word_R_RM,
  /*  0x1c */ x86emuOp_op_A_byte_AL_IMM,
  /*  0x1d */ x86emuOp_op_A_word_AX_IMM,
  /*  0x1e */ x86emuOp_push_DS,
  /*  0x1f */ x86emuOp_pop_DS,

  /*  0x20 */ x86emuOp_op_A_byte_RM_R,
  /*  0x21 */ x86emuOp_op_A_word_RM_R,
  /*  0x22 */ x86emuOp_op_A_byte_R_RM,
  /*  0x23 */ x86emuOp_op_A_word_R_RM,
  /*  0x24 */ x86emuOp_op_A_byte_AL_IMM,
  /*  0x25 */ x86emuOp_op_A_word_AX_IMM,
  /*  0x26 */ x86emuOp_illegal_op,	/* ES: */
  /*  0x27 */ x86emuOp_daa,

  /*  0x28 */ x86emuOp_op_A_byte_RM_R,
  /*  0x29 */ x86emuOp_op_A_word_RM_R,
  /*  0x2a */ x86emuOp_op_A_byte_R_RM,
  /*  0x2b */ x86emuOp_op_A_word_R_RM,
  /*  0x2c */ x86emuOp_op_A_byte_AL_IMM,
  /*  0x2d */ x86emuOp_op_A_word_AX_IMM,
  /*  0x2e */ x86emuOp_illegal_op,	/* CS: */
  /*  0x2f */ x86emuOp_das,

  /*  0x30 */ x86emuOp_op_A_byte_RM_R,
  /*  0x31 */ x86emuOp_op_A_word_RM_R,
  /*  0x32 */ x86emuOp_op_A_byte_R_RM,
  /*  0x33 */ x86emuOp_op_A_word_R_RM,
  /*  0x34 */ x86emuOp_op_A_byte_AL_IMM,
  /*  0x35 */ x86emuOp_op_A_word_AX_IMM,
  /*  0x36 */ x86emuOp_illegal_op,	/* SS: */
  /*  0x37 */ x86emuOp_aaa,

  /*  0x38 */ x86emuOp_op_A_byte_RM_R,
  /*  0x39 */ x86emuOp_op_A_word_RM_R,
  /*  0x3a */ x86emuOp_op_A_byte_R_RM,
  /*  0x3b */ x86emuOp_op_A_word_R_RM,
  /*  0x3c */ x86emuOp_op_A_byte_AL_IMM,
  /*  0x3d */ x86emuOp_op_A_word_AX_IMM,
  /*  0x3e */ x86emuOp_illegal_op,	/* DS: */
  /*  0x3f */ x86emuOp_aas,

  /*  0x40 */ x86emuOp_inc_AX,
  /*  0x41 */ x86emuOp_inc_CX,
  /*  0x42 */ x86emuOp_inc_DX,
  /*  0x43 */ x86emuOp_inc_BX,
  /*  0x44 */ x86emuOp_inc_SP,
  /*  0x45 */ x86emuOp_inc_BP,
  /*  0x46 */ x86emuOp_inc_SI,
  /*  0x47 */ x86emuOp_inc_DI,

  /*  0x48 */ x86emuOp_dec_AX,
  /*  0x49 */ x86emuOp_dec_CX,
  /*  0x4a */ x86emuOp_dec_DX,
  /*  0x4b */ x86emuOp_dec_BX,
  /*  0x4c */ x86emuOp_dec_SP,
  /*  0x4d */ x86emuOp_dec_BP,
  /*  0x4e */ x86emuOp_dec_SI,
  /*  0x4f */ x86emuOp_dec_DI,

  /*  0x50 */ x86emuOp_push_AX,
  /*  0x51 */ x86emuOp_push_CX,
  /*  0x52 */ x86emuOp_push_DX,
  /*  0x53 */ x86emuOp_push_BX,
  /*  0x54 */ x86emuOp_push_SP,
  /*  0x55 */ x86emuOp_push_BP,
  /*  0x56 */ x86emuOp_push_SI,
  /*  0x57 */ x86emuOp_push_DI,

  /*  0x58 */ x86emuOp_pop_AX,
  /*  0x59 */ x86emuOp_pop_CX,
  /*  0x5a */ x86emuOp_pop_DX,
  /*  0x5b */ x86emuOp_pop_BX,
  /*  0x5c */ x86emuOp_pop_SP,
  /*  0x5d */ x86emuOp_pop_BP,
  /*  0x5e */ x86emuOp_pop_SI,
  /*  0x5f */ x86emuOp_pop_DI,

  /*  0x60 */ x86emuOp_push_all,
  /*  0x61 */ x86emuOp_pop_all,
  /*  0x62 */ x86emuOp_illegal_op,	/* bound */
  /*  0x63 */ x86emuOp_illegal_op,	/* arpl */
  /*  0x64 */ x86emuOp_illegal_op,	/* FS: */
  /*  0x65 */ x86emuOp_illegal_op,	/* GS: */
  /*  0x66 */ x86emuOp_illegal_op,	/* DATA32: */
  /*  0x67 */ x86emuOp_illegal_op,	/* ADDR32: */

  /*  0x68 */ x86emuOp_push_word_IMM,
  /*  0x69 */ x86emuOp_imul_word_IMM,
  /*  0x6a */ x86emuOp_push_byte_IMM,
  /*  0x6b */ x86emuOp_imul_byte_IMM,
  /*  0x6c */ x86emuOp_ins_byte,
  /*  0x6d */ x86emuOp_ins_word,
  /*  0x6e */ x86emuOp_outs_byte,
  /*  0x6f */ x86emuOp_outs_word,

  /*  0x70 */ x86emuOp_jump_short_cc,
  /*  0x71 */ x86emuOp_jump_short_cc,
  /*  0x72 */ x86emuOp_jump_short_cc,
  /*  0x73 */ x86emuOp_jump_short_cc,
  /*  0x74 */ x86emuOp_jump_short_cc,
  /*  0x75 */ x86emuOp_jump_short_cc,
  /*  0x76 */ x86emuOp_jump_short_cc,
  /*  0x77 */ x86emuOp_jump_short_cc,

  /*  0x78 */ x86emuOp_jump_short_cc,
  /*  0x79 */ x86emuOp_jump_short_cc,
  /*  0x7a */ x86emuOp_jump_short_cc,
  /*  0x7b */ x86emuOp_jump_short_cc,
  /*  0x7c */ x86emuOp_jump_short_cc,
  /*  0x7d */ x86emuOp_jump_short_cc,
  /*  0x7e */ x86emuOp_jump_short_cc,
  /*  0x7f */ x86emuOp_jump_short_cc,

  /*  0x80 */ x86emuOp_opc80_byte_RM_IMM,
  /*  0x81 */ x86emuOp_opc81_word_RM_IMM,
  /*  0x82 */ x86emuOp_opc80_byte_RM_IMM,
  /*  0x83 */ x86emuOp_opc83_word_RM_IMM,
  /*  0x84 */ x86emuOp_test_byte_RM_R,
  /*  0x85 */ x86emuOp_test_word_RM_R,
  /*  0x86 */ x86emuOp_xchg_byte_RM_R,
  /*  0x87 */ x86emuOp_xchg_word_RM_R,

  /*  0x88 */ x86emuOp_mov_byte_RM_R,
  /*  0x89 */ x86emuOp_mov_word_RM_R,
  /*  0x8a */ x86emuOp_mov_byte_R_RM,
  /*  0x8b */ x86emuOp_mov_word_R_RM,
  /*  0x8c */ x86emuOp_mov_word_RM_SR,
  /*  0x8d */ x86emuOp_lea_word_R_M,
  /*  0x8e */ x86emuOp_mov_word_SR_RM,
  /*  0x8f */ x86emuOp_pop_RM,

  /*  0x90 */ x86emuOp_nop,
  /*  0x91 */ x86emuOp_xchg_word_AX_CX,
  /*  0x92 */ x86emuOp_xchg_word_AX_DX,
  /*  0x93 */ x86emuOp_xchg_word_AX_BX,
  /*  0x94 */ x86emuOp_xchg_word_AX_SP,
  /*  0x95 */ x86emuOp_xchg_word_AX_BP,
  /*  0x96 */ x86emuOp_xchg_word_AX_SI,
  /*  0x97 */ x86emuOp_xchg_word_AX_DI,

  /*  0x98 */ x86emuOp_cbw,
  /*  0x99 */ x86emuOp_cwd,
  /*  0x9a */ x86emuOp_call_far_IMM,
  /*  0x9b */ x86emuOp_wait,
  /*  0x9c */ x86emuOp_pushf_word,
  /*  0x9d */ x86emuOp_popf_word,
  /*  0x9e */ x86emuOp_sahf,
  /*  0x9f */ x86emuOp_lahf,

  /*  0xa0 */ x86emuOp_mov_AL_M_IMM,
  /*  0xa1 */ x86emuOp_mov_AX_M_IMM,
  /*  0xa2 */ x86emuOp_mov_M_AL_IMM,
  /*  0xa3 */ x86emuOp_mov_M_AX_IMM,
  /*  0xa4 */ x86emuOp_movs_byte,
  /*  0xa5 */ x86emuOp_movs_word,
  /*  0xa6 */ x86emuOp_cmps_byte,
  /*  0xa7 */ x86emuOp_cmps_word,
  /*  0xa8 */ x86emuOp_test_AL_IMM,
  /*  0xa9 */ x86emuOp_test_AX_IMM,
  /*  0xaa */ x86emuOp_stos_byte,
  /*  0xab */ x86emuOp_stos_word,
  /*  0xac */ x86emuOp_lods_byte,
  /*  0xad */ x86emuOp_lods_word,
  /*  0xac */ x86emuOp_scas_byte,
  /*  0xad */ x86emuOp_scas_word,


  /*  0xb0 */ x86emuOp_mov_byte_AL_IMM,
  /*  0xb1 */ x86emuOp_mov_byte_CL_IMM,
  /*  0xb2 */ x86emuOp_mov_byte_DL_IMM,
  /*  0xb3 */ x86emuOp_mov_byte_BL_IMM,
  /*  0xb4 */ x86emuOp_mov_byte_AH_IMM,
  /*  0xb5 */ x86emuOp_mov_byte_CH_IMM,
  /*  0xb6 */ x86emuOp_mov_byte_DH_IMM,
  /*  0xb7 */ x86emuOp_mov_byte_BH_IMM,

  /*  0xb8 */ x86emuOp_mov_word_AX_IMM,
  /*  0xb9 */ x86emuOp_mov_word_CX_IMM,
  /*  0xba */ x86emuOp_mov_word_DX_IMM,
  /*  0xbb */ x86emuOp_mov_word_BX_IMM,
  /*  0xbc */ x86emuOp_mov_word_SP_IMM,
  /*  0xbd */ x86emuOp_mov_word_BP_IMM,
  /*  0xbe */ x86emuOp_mov_word_SI_IMM,
  /*  0xbf */ x86emuOp_mov_word_DI_IMM,

  /*  0xc0 */ x86emuOp_opcC0_byte_RM_MEM,
  /*  0xc1 */ x86emuOp_opcC1_word_RM_MEM,
  /*  0xc2 */ x86emuOp_ret_near_IMM,
  /*  0xc3 */ x86emuOp_ret_near,
  /*  0xc4 */ x86emuOp_les_R_IMM,
  /*  0xc5 */ x86emuOp_lds_R_IMM,
  /*  0xc6 */ x86emuOp_mov_byte_RM_IMM,
  /*  0xc7 */ x86emuOp_mov_word_RM_IMM,
  /*  0xc8 */ x86emuOp_enter,
  /*  0xc9 */ x86emuOp_leave,
  /*  0xca */ x86emuOp_ret_far_IMM,
  /*  0xcb */ x86emuOp_ret_far,
  /*  0xcc */ x86emuOp_int3,
  /*  0xcd */ x86emuOp_int_IMM,
  /*  0xce */ x86emuOp_into,
  /*  0xcf */ x86emuOp_iret,

  /*  0xd0 */ x86emuOp_opcD0_byte_RM_1,
  /*  0xd1 */ x86emuOp_opcD1_word_RM_1,
  /*  0xd2 */ x86emuOp_opcD2_byte_RM_CL,
  /*  0xd3 */ x86emuOp_opcD3_word_RM_CL,
  /*  0xd4 */ x86emuOp_aam,
  /*  0xd5 */ x86emuOp_aad,
  /*  0xd6 */ x86emuOp_setalc,
  /*  0xd7 */ x86emuOp_xlat,

#if 0
  /*  0xd8 */ x86emuOp_esc_coprocess_d8,
  /*  0xd9 */ x86emuOp_esc_coprocess_d9,
  /*  0xda */ x86emuOp_esc_coprocess_da,
  /*  0xdb */ x86emuOp_esc_coprocess_db,
  /*  0xdc */ x86emuOp_esc_coprocess_dc,
  /*  0xdd */ x86emuOp_esc_coprocess_dd,
  /*  0xde */ x86emuOp_esc_coprocess_de,
  /*  0xdf */ x86emuOp_esc_coprocess_df,
#else
  /*  0xd8 */ x86emuOp_illegal_op,
  /*  0xd9 */ x86emuOp_illegal_op,
  /*  0xda */ x86emuOp_illegal_op,
  /*  0xdb */ x86emuOp_illegal_op,
  /*  0xdc */ x86emuOp_illegal_op,
  /*  0xdd */ x86emuOp_illegal_op,
  /*  0xde */ x86emuOp_illegal_op,
  /*  0xdf */ x86emuOp_illegal_op,
#endif

  /*  0xe0 */ x86emuOp_loopne,
  /*  0xe1 */ x86emuOp_loope,
  /*  0xe2 */ x86emuOp_loop,
  /*  0xe3 */ x86emuOp_jcxz,
  /*  0xe4 */ x86emuOp_in_byte_AL_IMM,
  /*  0xe5 */ x86emuOp_in_word_AX_IMM,
  /*  0xe6 */ x86emuOp_out_byte_IMM_AL,
  /*  0xe7 */ x86emuOp_out_word_IMM_AX,

  /*  0xe8 */ x86emuOp_call_near_IMM,
  /*  0xe9 */ x86emuOp_jump_near_IMM,
  /*  0xea */ x86emuOp_jump_far_IMM,
  /*  0xeb */ x86emuOp_jump_byte_IMM,
  /*  0xec */ x86emuOp_in_byte_AL_DX,
  /*  0xed */ x86emuOp_in_word_AX_DX,
  /*  0xee */ x86emuOp_out_byte_DX_AL,
  /*  0xef */ x86emuOp_out_word_DX_AX,

  /*  0xf0 */ x86emuOp_illegal_op,	/* LOCK: */
  /*  0xf1 */ x86emuOp_illegal_op,
  /*  0xf2 */ x86emuOp_illegal_op,	/* REPNE: */
  /*  0xf3 */ x86emuOp_illegal_op,	/* REPE: */
  /*  0xf4 */ x86emuOp_hlt,
  /*  0xf5 */ x86emuOp_cmc,
  /*  0xf6 */ x86emuOp_opcF6_byte_RM,
  /*  0xf7 */ x86emuOp_opcF7_word_RM,

  /*  0xf8 */ x86emuOp_clc,
  /*  0xf9 */ x86emuOp_stc,
  /*  0xfa */ x86emuOp_cli,
  /*  0xfb */ x86emuOp_sti,
  /*  0xfc */ x86emuOp_cld,
  /*  0xfd */ x86emuOp_std,
  /*  0xfe */ x86emuOp_opcFE_byte_RM,
  /*  0xff */ x86emuOp_opcFF_word_RM,
};

