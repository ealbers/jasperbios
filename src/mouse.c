// 16bit code to handle mouse events.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "biosvar.h" // GET_EBDA
#include "bregs.h" // struct bregs
#include "hw/usb-hid.h" // usb_mouse_command
#include "output.h" // dprintf
#include "stacks.h" // stack_hop_back
#include "util.h" // mouse_init

void
mouse_init(void)
{
    if (! CONFIG_MOUSE)
        return;
    dprintf(3, "init mouse\n");
    // pointing device installed
    set_equipment_flags(0x04, 0x04);
}

static int
mouse_command(int command, u8 *param)
{
   
        return usb_mouse_command(command, param);
    
}

#define RET_SUCCESS      0x00
#define RET_EINVFUNCTION 0x01
#define RET_EINVINPUT    0x02
#define RET_EINTERFACE   0x03
#define RET_ENEEDRESEND  0x04
#define RET_ENOHANDLER   0x05

// Disable Mouse
static void
mouse_15c20000(struct bregs *regs)
{
    set_code_success(regs);
}

// Enable Mouse
static void
mouse_15c20001(struct bregs *regs)
{
    set_code_success(regs);
}

static void
mouse_15c200XX(struct bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Disable/Enable Mouse
static void
mouse_15c200(struct bregs *regs)
{
    switch (regs->bh) {
    case 0x00: mouse_15c20000(regs); break;
    case 0x01: mouse_15c20001(regs); break;
    default:   mouse_15c200XX(regs); break;
    }
}

// Reset Mouse
static void
mouse_15c201(struct bregs *regs)
{
   
    set_code_success(regs);
}

// Set Sample Rate
static void
mouse_15c202(struct bregs *regs)
{
    
        set_code_success(regs);
}

// Set Resolution
static void
mouse_15c203(struct bregs *regs)
{
    
        set_code_success(regs);
}

// Get Device ID
static void
mouse_15c204(struct bregs *regs)
{
   
    set_code_success(regs);
}

// Initialize Mouse
static void
mouse_15c205(struct bregs *regs)
{
    
    mouse_15c201(regs);
}

// Return Status
static void
mouse_15c20600(struct bregs *regs)
{
    
    set_code_success(regs);
}

// Set Scaling Factor to 1:1
static void
mouse_15c20601(struct bregs *regs)
{
    
        set_code_success(regs);
}

// Set Scaling Factor to 2:1
static void
mouse_15c20602(struct bregs *regs)
{
    
        set_code_success(regs);
}

static void
mouse_15c206XX(struct bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Return Status & Set Scaling Factor...
static void
mouse_15c206(struct bregs *regs)
{
    switch (regs->bh) {
    case 0x00: mouse_15c20600(regs); break;
    case 0x01: mouse_15c20601(regs); break;
    case 0x02: mouse_15c20602(regs); break;
    default:   mouse_15c206XX(regs); break;
    }
}

// Set Mouse Handler Address
static void
mouse_15c207(struct bregs *regs)
{
   
    set_code_success(regs);
}

static void
mouse_15c2XX(struct bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

void
handle_15c2(struct bregs *regs)
{
    //debug_stub(regs);

    if (! CONFIG_MOUSE) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        return;
    }

    switch (regs->al) {
    case 0x00: mouse_15c200(regs); break;
    case 0x01: mouse_15c201(regs); break;
    case 0x02: mouse_15c202(regs); break;
    case 0x03: mouse_15c203(regs); break;
    case 0x04: mouse_15c204(regs); break;
    case 0x05: mouse_15c205(regs); break;
    case 0x06: mouse_15c206(regs); break;
    case 0x07: mouse_15c207(regs); break;
    default:   mouse_15c2XX(regs); break;
    }
}

void VISIBLE16
invoke_mouse_handler(void)
{
    if (!CONFIG_MOUSE)
        return;
    if (need_hop_back()) {
        stack_hop_back(invoke_mouse_handler, 0, 0);
        return;
    }
    ASSERT16();
    u16 ebda_seg = get_ebda_seg();
    u16 status = GET_EBDA(ebda_seg, mouse_data[0]);
    u16 X      = GET_EBDA(ebda_seg, mouse_data[1]);
    u16 Y      = GET_EBDA(ebda_seg, mouse_data[2]);

    struct segoff_s func = GET_EBDA(ebda_seg, far_call_pointer);
    dprintf(16, "mouse farcall s=%04x x=%04x y=%04x func=%04x:%04x\n"
            , status, X, Y, func.seg, func.offset);

    asm volatile(
        "pushl %%ebp\n"
        "sti\n"

        "pushl %0\n"
        "pushw %w1\n"  // status
        "pushw %w2\n"  // X
        "pushw %w3\n"  // Y
        "pushw $0\n"   // Z
        "lcallw *8(%%esp)\n"
        "addl $12, %%esp\n"

        "cli\n"
        "cld\n"
        "popl %%ebp"
        : "+a"(func.segoff), "+c"(status), "+d"(X), "+b"(Y)
        :
        : "edi", "esi", "cc", "memory");
}

void
process_mouse(u8 data)
{
    if (!CONFIG_MOUSE)
        return;

    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_1 = GET_EBDA(ebda_seg, mouse_flag1);
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);

    if (! (mouse_flags_2 & 0x80))
        // far call handler not installed
        return;

    u8 package_count = mouse_flags_2 & 0x07;
    u8 index = mouse_flags_1 & 0x07;
    SET_EBDA(ebda_seg, mouse_data[index], data);

    if ((index+1) < package_count) {
        mouse_flags_1++;
        SET_EBDA(ebda_seg, mouse_flag1, mouse_flags_1);
        return;
    }

    SET_EBDA(ebda_seg, mouse_flag1, 0);
    invoke_mouse_handler();
}
