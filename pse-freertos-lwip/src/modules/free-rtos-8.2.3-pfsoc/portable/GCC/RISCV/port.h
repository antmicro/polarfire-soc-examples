
#ifndef PORTABLE_GCC_RISCV_PORT_H_
#define PORTABLE_GCC_RISCV_PORT_H_

/*
 * Handler for timer interrupt
 */
void vPortSysTickHandler( void );

/*
 * Setup the timer to generate the tick interrupts.
 */
void vPortSetupTimer( void );

/*
 * Set the next interval for the timer
 */
static void prvSetNextTimerInterrupt( void );

/*
 * Used to catch tasks that attempt to return from their implementing function.
 */
static void prvTaskExitError( void );


#endif /* PORTABLE_GCC_RISCV_PORT_H_ */
