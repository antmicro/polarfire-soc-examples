# See LICENSE for license details.

#include "bits.h"
#include "encoding.h"
#include "mtrap.h"

#define BAD_TRAP_VECTOR                (0)
#define TRAP_FROM_MACHINE_MODE_VECTOR (13)

  .data
  .align 6
trap_table:                         # Description from Volume II: RISC-V Privileged Architectures V1.10-draft
  .word bad_trap                    # Insruction address misaligned
  .word pmp_trap1                   # Instruction access fault
  .word illegal_insn_trap           # Illegal Instruction
  .word bad_trap                    # Breakpoint
  .word misaligned_load_trap        # Load address misaligned
  .word pmp_trap2                   # Load access fault
  .word misaligned_store_trap       # Store/AMO address misaligned
  .word pmp_trap3                   # Store/AMO access fault
  .word bad_trap                    # Environment call from U-mode
  .word mcall_trap                  # Environment call from S-mode
  .word bad_trap                    # Environment call from H-mode
  .word bad_trap                    # Environment call from M-mode
  .word bad_trap                    # Instruction page fault
  .word __trap_from_machine_mode    # Load page fault
  .word bad_trap                    # Reserved
  .word bad_trap                    # Store/AMO page fault

  .option norvc
  .section .text.init,"ax",@progbits
  .globl reset_vector
  .globl _start
  reset_vector:
_start:
  j do_reset

.Le51_trap:
    csrr  t0, mcause
    bge   t0,x0,.Le51_other  /* Not an interrupt... */
    slli  t0,t0,1
    srli  t0,t0,1
    addi  t0,t0,-7
    bne   t0,x0,.Le51_other /* Not Timer interrupt... */

    /* Interrupt is timer interrupt */
    LOAD    t0, 0x0(sp)
    addi    sp, sp, REGBYTES
    j       TIMER_CMP_INT
    mret


trap_vector:
  addi    sp, sp, -REGBYTES
  STORE   t0, 0x0(sp)
  csrr    t0, mhartid
  beqz    t0, .Le51_trap    # Treat e51 differently...

.Le51_other:                # Re-enter mainline here if not e51 timer interrupt
  LOAD    t0, 0x0(sp)
  addi    sp, sp, REGBYTES

  csrrw sp, mscratch, sp            # Exchange SP and mscratch
  beqz sp, .Ltrap_from_machine_mode # If mscratch was 0

  STORE a0, 10*REGBYTES(sp)
  STORE a1, 11*REGBYTES(sp)

  csrr a1, mcause
  bgez a1, .Lhandle_trap_in_machine_mode

  # This is an interrupt.  Discard the mcause MSB and decode the rest.
  sll a1, a1, 1

  # Is it a machine timer interrupt?
  li a0, IRQ_M_TIMER * 2
  bne a0, a1, 1f

  # Yes. Simply clear MSIE and raise SSIP
  li a0, MIP_MTIP
  csrc mie, a0
  li a0, MIP_STIP
  csrs mip, a0

.Lmret:
  # Go back to whence we came.
  LOAD a0, 10*REGBYTES(sp)
  LOAD a1, 11*REGBYTES(sp)
  csrrw sp, mscratch, sp
  mret

1:
  # Is it an IPI?
  li a0, IRQ_M_SOFT * 2
  bne a0, a1, .Lbad_trap

  # Yes. First clear the MIPI bit.
  LOAD a0, MENTRY_IPI_OFFSET(sp)
  sw x0, (a0)
  fence

  # Now, decode the cause(s).
#ifdef __riscv_atomic
  addi a0, sp, MENTRY_IPI_PENDING_OFFSET
  amoswap.w a0, x0, (a0)
#else
# error "only supporting atomic right now"
#endif
  and a1, a0, IPI_SOFT
  beqz a1, 1f
  csrs mip, MIP_SSIP
1:
  andi a1, a0, IPI_FENCE_I
  beqz a1, 1f
  fence.i
1:
  andi a1, a0, IPI_SFENCE_VMA
  beqz a1, 1f
  # sfence.vma - TODO
1:
  j .Lmret

.Lhandle_trap_in_machine_mode:
  # Preserve the registers.  Compute the address of the trap handler.
  STORE ra, 1*REGBYTES(sp)
  STORE gp, 3*REGBYTES(sp)
  STORE tp, 4*REGBYTES(sp)
  STORE t0, 5*REGBYTES(sp)
1:auipc t0, %pcrel_hi(trap_table)  # t0 <- %hi(trap_table)
  STORE t1, 6*REGBYTES(sp)
  sll t1, a1, 2                    # t1 <- mcause << 2
  STORE t2, 7*REGBYTES(sp)
  add t1, t0, t1                   # t1 <- %hi(trap_table)[mcause]
  STORE s0, 8*REGBYTES(sp)
  LWU t1, %pcrel_lo(1b)(t1)         # t1 <- trap_table[mcause]
  STORE s1, 9*REGBYTES(sp)
  mv a0, sp                        # a0 <- regs
  STORE a2,12*REGBYTES(sp)
  csrr a2, mepc                    # a2 <- mepc
  STORE a3,13*REGBYTES(sp)
  csrrw t0, mscratch, x0           # t0 <- user sp
  STORE a4,14*REGBYTES(sp)
  STORE a5,15*REGBYTES(sp)
  STORE a6,16*REGBYTES(sp)
  STORE a7,17*REGBYTES(sp)
  STORE s2,18*REGBYTES(sp)
  STORE s3,19*REGBYTES(sp)
  STORE s4,20*REGBYTES(sp)
  STORE s5,21*REGBYTES(sp)
  STORE s6,22*REGBYTES(sp)
  STORE s7,23*REGBYTES(sp)
  STORE s8,24*REGBYTES(sp)
  STORE s9,25*REGBYTES(sp)
  STORE s10,26*REGBYTES(sp)
  STORE s11,27*REGBYTES(sp)
  STORE t3,28*REGBYTES(sp)
  STORE t4,29*REGBYTES(sp)
  STORE t5,30*REGBYTES(sp)
  STORE t6,31*REGBYTES(sp)
  STORE t0, 2*REGBYTES(sp)         # sp

#ifndef __riscv_flen
#  lw tp, (sp) # Move the emulated FCSR from x0's save slot into tp.
#endif
  STORE x0, (sp) # Zero x0's save slot.

  # Invoke the handler.
  jalr t1

#ifndef __riscv_flen
#  sw tp, (sp) # Move the emulated FCSR from tp into x0's save slot.
#endif

restore_mscratch:
  # Restore mscratch, so future traps will know they didn't come from M-mode.
  csrw mscratch, sp

restore_regs:
  # Restore all of the registers.
  LOAD ra, 1*REGBYTES(sp)
  LOAD gp, 3*REGBYTES(sp)
  LOAD tp, 4*REGBYTES(sp)
  LOAD t0, 5*REGBYTES(sp)
  LOAD t1, 6*REGBYTES(sp)
  LOAD t2, 7*REGBYTES(sp)
  LOAD s0, 8*REGBYTES(sp)
  LOAD s1, 9*REGBYTES(sp)
  LOAD a0,10*REGBYTES(sp)
  LOAD a1,11*REGBYTES(sp)
  LOAD a2,12*REGBYTES(sp)
  LOAD a3,13*REGBYTES(sp)
  LOAD a4,14*REGBYTES(sp)
  LOAD a5,15*REGBYTES(sp)
  LOAD a6,16*REGBYTES(sp)
  LOAD a7,17*REGBYTES(sp)
  LOAD s2,18*REGBYTES(sp)
  LOAD s3,19*REGBYTES(sp)
  LOAD s4,20*REGBYTES(sp)
  LOAD s5,21*REGBYTES(sp)
  LOAD s6,22*REGBYTES(sp)
  LOAD s7,23*REGBYTES(sp)
  LOAD s8,24*REGBYTES(sp)
  LOAD s9,25*REGBYTES(sp)
  LOAD s10,26*REGBYTES(sp)
  LOAD s11,27*REGBYTES(sp)
  LOAD t3,28*REGBYTES(sp)
  LOAD t4,29*REGBYTES(sp)
  LOAD t5,30*REGBYTES(sp)
  LOAD t6,31*REGBYTES(sp)
  LOAD sp, 2*REGBYTES(sp)
  mret

.Ltrap_from_machine_mode:
  csrr sp, mscratch                     # SP and mscratch now equal to original entry SP
  addi sp, sp, -INTEGER_CONTEXT_SIZE
  STORE a0, 10*REGBYTES(sp)
  STORE a1, 11*REGBYTES(sp)
  li a1, TRAP_FROM_MACHINE_MODE_VECTOR
  j .Lhandle_trap_in_machine_mode

.Lbad_trap:
  li a1, BAD_TRAP_VECTOR
  j .Lhandle_trap_in_machine_mode

  .globl __redirect_trap
__redirect_trap:
  # Reset sp to top of M-Mode stack
  li t0, MACHINE_STACK_SIZE
  add sp, sp, t0
  neg t0, t0
  and sp, sp, t0
  addi sp, sp, -MENTRY_FRAME_SIZE
  j restore_mscratch

__trap_from_machine_mode:
  jal trap_from_machine_mode
  j restore_regs

do_reset:
  la t0, trap_vector
  csrw mtvec, t0

  li  x1, 0
  li  x2, 0
  li  x3, 0
  li  x4, 0
  li  x5, 0
  li  x6, 0
  li  x7, 0
  li  x8, 0
  li  x9, 0
  li  x10,0
  li  x11,0
  li  x12,0
  li  x13,0
  li  x14,0
  li  x15,0
  li  x16,0
  li  x17,0
  li  x18,0
  li  x19,0
  li  x20,0
  li  x21,0
  li  x22,0
  li  x23,0
  li  x24,0
  li  x25,0
  li  x26,0
  li  x27,0
  li  x28,0
  li  x29,0
  li  x30,0
  li  x31,0

  # enable FPU and accelerator if present
  li t0, MSTATUS_FS | MSTATUS_XS
  csrs mstatus, t0

  # make sure XLEN agrees with compilation choice
  csrr t0, misa
#if __riscv_xlen == 64
  bltz t0, .LxlenCheck
#else
  bgez t0, .LxlenCheck
#endif
.LxlenCheck:

  # initialize global pointer
  la gp, _gp

#  la  tp, _end + 63
  # align stack bottom on 64 byte boundary
  la  tp, __stack_bottom + 63
  and tp, tp, -64

  # get core id
  csrr a0, mhartid

  # give each core 128KB of stack + TLS
# #define STKSHIFT 17
  #changed to 1K for the emulation platform
  #define STKSHIFT 12
  #if 1
  sll a2, a0, STKSHIFT
  add tp, tp, a2
  add sp, a0, 1
  sll sp, sp, STKSHIFT
  add sp, sp, tp
  #else
  sll a2, a0, STKSHIFT
  add tp, tp, a2
  li  a2, 1 << STKSHIFT
  add sp, tp, a2
  #endif
  # get core id
  csrr a0, mhartid
  li a1, 1
  bgeu a0, a1, .LbootOtherHart
  # core 0 jumps to main_first_hart
 # Enable this code to see the e51 startup run in user mode.
 # It wont get very far...
 #if 0
    csrci mstatus, MSTATUS_MIE      # Disable interrupts
    LA  t0, main_first_hart        # Set up mepc with address of user mode function
    csrw    mepc, t0
    li      t0, MSTATUS_MPP
    csrc    mstatus, t0         # Set previous mode to user
    li      t0, MSTATUS_MPIE
    csrs    mstatus, t0         # Make sure ints are renabled when we switch
    li      t0, MIP_MEIP
    csrc    mie, t0             # Disable external ints for the moment
    li      t0, 0x1F            # NAPOT region with RWX permission
    csrw    pmpcfg0,t0
    li      t0, 0xFFFFFFFF      # Select big area
    csrw    pmpaddr0,t0
    mret                        # Off to user land we go
#endif

  j main_first_hart

.LendFirstHart:
  # while 1
  # L below added by maalchy, remove
#  j main_other_hart
#  j .LendFirstHart

.LbootOtherHart:
  # L below added by maalchy, remove
  #j main_other_hart
  # core 1 jumps to main_other_hart
  # Set MSIE bit to receive IPI
  li a2, MIP_MSIP
  csrw mie, a2

.LwaitOtherHart:
  # Wait for an IPI to signal that its safe to boot
  #malachy fixme added code below while debugging
  #csrc sstatus, SSTATUS_SIE
  #csrs sie, SIP_SSIP
  #csrs sip, SIP_SSIP
#  set_csr(mstatus, MSTATUS_MIE);
  csrs mstatus, MSTATUS_MIE;
  csrs mie, MIP_MSIP;
  #malachy  fixme added code above while debugging
  wfi
  j main_other_hart
  # Only start if MIP_MSIP is set
  csrr a2, mip
  andi a2, a2, MIP_MSIP
  beqz a2, .LwaitOtherHart
  #j .LendFirstHart
  j main_other_hart

.LendOtherHart:
  # while 1
  j .LendOtherHart

.section ".tdata.begin"
.globl _tdata_begin
_tdata_begin:

.section ".tdata.end"
.globl _tdata_end
_tdata_end:

.section ".tbss.end"
.globl _tbss_end
_tbss_end:

# .section ".tohost","aw",@progbits
# .align 6
# .globl tohost
# tohost: .dword 0
# .align 6

# .globl fromhost
# fromhost: .dword 0

