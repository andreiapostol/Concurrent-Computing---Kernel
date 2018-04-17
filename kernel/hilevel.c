/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"

pcb_t pcb[ 64 ]; int executing = 0;
pipe_t pipes[ 64 ];
int numberOfProcesses = 1; int nextProcess = 0;
int currentPipeID = -1;
int currentPID = 0;
int offset = 0x00001000;

void customWrite(char *str, int length){
  for(int i = 0; i < length; i++){
    PL011_putc(UART0, str[i], true);
  }
}

void renderScreen(){
  customWrite("┏━━━━┳━━━━━━┳━━━━━━━━━━━━━┳━━━━━━━━━━━━┓",120);
  customWrite("\n", 1);
  customWrite("┃ NO ┃  PID ┃   STATUS    ┃  PRIORITY  ┃",50);
  customWrite("\n", 1);
  customWrite("┣━━━━╋━━━━━━╋━━━━━━━━━━━━━╋━━━━━━━━━━━━┫",120);
  customWrite("\n", 1);
  for(int i = 0; i < numberOfProcesses; i++){
    customWrite("┃ ", 4);
    printNumber(i);
    if(i < 10) customWrite("  ┃  ", 8);
    else customWrite(" ┃  ", 7);

    printNumber(pcb[i].pid);
    if(pcb[i].pid < 10) customWrite("   ┃ ", 8);
    else customWrite("  ┃ ", 7);

    switch(pcb[i].status){
      case STATUS_TERMINATED:
        customWrite("\x1B[31m TERMINATED\033[0m ┃     ", 30);
        break;
      case STATUS_READY:
        customWrite("\x1B[33m   READY\033[0m    ┃     ", 30);
        break;
      case STATUS_EXECUTING:
        customWrite("\x1B[32m EXECUTING\033[0m  ┃     ", 30);
      default:
        break;
    }

    printNumber(pcb[i].currentPriority);
    if(pcb[i].currentPriority < 10) customWrite("      ┃", 10);
    else if(pcb[i].currentPriority < 100)  customWrite("     ┃", 9);
    else if(pcb[i].currentPriority < 1000) customWrite("    ┃", 8);

    customWrite("\n", 1);
  }
    customWrite("┗━━━━┻━━━━━━┻━━━━━━━━━━━━━┻━━━━━━━━━━━━┛",120);
    customWrite("\n", 1);
}

void clearScreen(){
  customWrite("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n", 15);
}

// else if (pcb[i].currentPriority == maxPriority){
//   pcb[i].currentPriority += pcb[i].basePriority / 10;
//   pcb[maxIndex].currentPriority = 0;
// }


/* Since we *know* there will be 2 processes, stemming from the 2 user
 * programs, we can
 *
 * - allocate a fixed-size process table (of PCBs), and then maintain
 *   an index into it for the currently executing process,
 * - employ a fixed-case of round-robin scheduling: no more processes
 *   can be created, and neither is able to terminate.
 */

// enum PipeStatus {UNITIALIZED};

void scheduler( ctx_t* ctx ) {
  // PL011_putc( UART0, '`', true );
  // PL011_putc( UART0, '0'+executing, true );
  // PL011_putc( UART0, '`', true );
  // PL011_putc( UART0, ' ', true );
    // PL011_putc( UART0, '[', true );
    // for(int i = 0; i < numberOfProcesses; i++)
    //   PL011_putc( UART0, '0' + (pcb[i].status == STATUS_TERMINATED ? 1 : 0), true );
    // PL011_putc( UART0, ']', true );
    // PL011_putc( UART0, ' ', true );
    if(numberOfProcesses != 1){
      int exExecuting = executing;
      do{
        nextProcess = (++exExecuting) % numberOfProcesses;
      }
      while(pcb[ nextProcess ].status == STATUS_TERMINATED);
    }
    else{
      nextProcess = executing;
    }

  // prioritize(pcb, numberOfProcesses);
  // int a  = getMaximumPriorityIndex(pcb, numberOfProcesses);
  // nextProcess = getMaximumPriorityIndex(pcb, numberOfProcesses);
  // if(numberOfProcesses == 2) nextProcess = 1;




  if(nextProcess != executing){
    memcpy( &pcb[ executing ].ctx, ctx, sizeof( ctx_t ) );        // preserve P_1

    if(pcb[ executing ].status != STATUS_TERMINATED)
      pcb[ executing ].status = STATUS_READY;                     // update   P_1 status

    memcpy( ctx, &pcb[ nextProcess ].ctx, sizeof( ctx_t ) );      // restore  P_2
    pcb[ nextProcess ].status = STATUS_EXECUTING;                 // update   P_2 status
    executing = nextProcess;                                      // update   index => P_2
  }
  // PL011_putc( UART0, 'E', true );
  // PL011_putc( UART0, '0'+executing, true );
  // PL011_putc( UART0, ':', true );
  // PL011_putc( UART0, pcb[ executing ].ctx.gpr[0], true );
  // write( STDOUT_FILENO, pcb[ executing ].ctx.gpr[0], 2 );
  return;
}

extern void main_console();
// extern uint32_t tos_console;
extern uint32_t tos_general;

void hilevel_handler_rst( ctx_t* ctx              ) {

  /* Initialise PCBs representing processes stemming from execution of
   * the two user programs.  Note in each case that
   *
   * - the CPSR value of 0x50 means the processor is switched into USR
   *   mode, with IRQ interrupts enabled, and
   * - the PC and SP values matche the entry point and top of stack.
   */

    PL011_putc( UART0, ' ', true );
    PL011_putc( UART0, 'R', true );
    PL011_putc( UART0, 'S', true );
    PL011_putc( UART0, 'T', true );

    memset( &pcb[ 0 ], 0, sizeof( pcb_t ) );
    pcb[ 0 ].pid      = 0;
    pcb[ 0 ].status   = STATUS_READY;
    pcb[ 0 ].ctx.cpsr = 0x50;
    pcb[ 0 ].ctx.pc   = ( uint32_t )( &main_console );
    pcb[ 0 ].ctx.sp   = ( uint32_t )( &(tos_general)  );
    pcb[ 0 ].tos   = ( uint32_t )( &(tos_general)  );
    pcb[ 0 ].basePriority = 100;
    pcb[ 0 ].currentPriority = 0;

    for(int i = 0; i < 10; i++){
      pcb[i].status = STATUS_TERMINATED;
    }

  /* Once the PCBs are initialised, we (arbitrarily) select one to be
   * restored (i.e., executed) when the function then returns.
   */

   memcpy( ctx, &pcb[ 0 ].ctx, sizeof( ctx_t ) );

    pcb[ 0 ].status = STATUS_EXECUTING;
    executing = 0;

    // TIMER0->Timer1Load  = 0x00011010; // select period = 2^20 ticks ~= 1 sec
    TIMER0->Timer1Load  = 0x00010000; // select period = 2^20 ticks ~= 1 sec
    TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
    TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
    TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
    TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

    GICC0->PMR          = 0x000000F0; // unmask all            interrupts
    GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
    GICC0->CTLR         = 0x00000001; // enable GIC interface
    GICD0->CTLR         = 0x00000001; // enable GIC distributor

    int_enable_irq();

    for(int i = 0; i < 64; i++){
      pipes[i].isWritten = 0;
      pipes[i].isClosed  = 0;
      pipes[i].isFree    = 1;
      pipes[i].message   = UNWRITTEN;
    }

    return;
}

void hilevel_handler_irq(ctx_t* ctx) {
  // Step 2: read  the interrupt identifier so we know the source.

  uint32_t id = GICC0->IAR;
  // PL011_putc( UART0, ' ', true );
  // PL011_putc( UART0, 'I', true );
  // PL011_putc( UART0, 'R', true );
  // PL011_putc( UART0, 'Q', true );
// Step 4: handle the interrupt, then clear (or reset) the source.

  if( id == GIC_SOURCE_TIMER0 ) {
    TIMER0->Timer1IntClr = 0x01;
    scheduler(ctx);
  }

// Step 5: write the interrupt identifier to signal we're done.

  GICC0->EOIR = id;
  customWrite("\e[1;1H\e[2J", 12);
  renderScreen();

  // printString("\\   ●  |  ●  |  ●  |  ●    /",28);
  // printString("\n",1);
  // printString("● ╔=======================╗ ●",29);
  // printString("\n",1);
  // printString("| ║                       ║ |",29);
  // printString("\n",1);
  // printString("  ╚=======================╝ ",29);
  // printString("\n",1);
  // printString("\\   ●  |  ●  |  ●  |  ●    /",30);

  // printString("\\  o   |  x  |  o  |   o   /\n", 29);
  // printString("o                           o\n", 30);
  // printString("|                           |\n", 30);
  // printString("o                           o\n", 30);
  // printString("|                           |\n", 30);
  // printString("o                           o\n", 30);
  // printString("|                           |\n", 30);
  // printString("o                           o\n", 30);
  // printString("/  o   |  o  |  o  |   o   \\\n", 29);
  // for(int i = 0; i < 8; i++){
  //   printNumber(pipes[i].message);
  //   printString(" ", 1);
  // }

  return;
}

void hilevel_handler_svc( ctx_t* ctx, uint32_t id ) {
  /* Based on the identified encoded as an immediate operand in the
   * instruction,
   *
   * - read  the arguments from preserved usr mode registers,
   * - perform whatever is appropriate for this system call,
   * - write any return value back to preserved usr mode registers.
   */

  switch( id ) {
    case 0x00 : { // 0x00 => yield()

      break;
    }

    case 0x01 : { // 0x01 => write( fd, x, n )
      int   fd = ( int   )( ctx->gpr[ 0 ] );
      char*  x = ( char* )( ctx->gpr[ 1 ] );
      int    n = ( int   )( ctx->gpr[ 2 ] );

      for( int i = 0; i < n; i++ ) {
        PL011_putc( UART0, *x++, true );
      }

      ctx->gpr[ 0 ] = n;
      // PL011_putc( UART0, 'B', true );
      break;
    }

    case 0x03: { // 0x03 => fork()
      // PL011_putc( UART0, 'F', true );
      // PL011_putc( UART0, 'O', true );
      // PL011_putc( UART0, 'R', true );
      // PL011_putc( UART0, 'K', true );
      // PL011_putc( UART0, ' ', true );

      int firstFreePosition = numberOfProcesses;
      for(int i = 0; i < numberOfProcesses; i++)
        if(pcb[ i ].status == STATUS_TERMINATED){
          firstFreePosition = i;
          break;
        }

      if(firstFreePosition == numberOfProcesses) numberOfProcesses++;
      currentPID++;

      int newTos = (firstFreePosition == numberOfProcesses - 1) ?
                   (uint32_t) pcb[ executing ].tos + (uint32_t) ((firstFreePosition) * offset) :
                   pcb[ firstFreePosition ].tos;

      memset(&pcb[firstFreePosition], 0, sizeof(pcb_t));
      pcb[ firstFreePosition ].pid      = currentPID;
      pcb[ firstFreePosition ].status   = STATUS_READY;

      memcpy(&(pcb[ firstFreePosition ].ctx), ctx, sizeof(ctx_t));

      pcb[ firstFreePosition ].ctx.gpr[ 0 ] = 0;
      // PL011_putc( UART0, '0' + (pcb[ firstFreePosition ].ctx.gpr[ 0 ] == 0) ? 1 : 0, true );
      // if (pcb[ firstFreePosition ].ctx.gpr[ 0 ] == 0)
      //   PL011_putc( UART0, 'Y', true );
      uint32_t offset_tos = (uint32_t) pcb[ executing ].tos - (uint32_t) ctx->sp;

      // if(firstFreePosition == numberOfProcesses-1)
        // pcb[ firstFreePosition ].tos = (uint32_t) pcb[ executing ].tos + (uint32_t) ((firstFreePosition) * offset);
      pcb[ firstFreePosition ].tos          =  newTos;
      pcb[ firstFreePosition ].ctx.sp       =  pcb[ firstFreePosition ].tos + offset_tos;
      // (&tos_general) + (executing * offset) + ctx->sp;

      memcpy((void*) pcb[ firstFreePosition ].tos, (void*) pcb[ executing ].tos , offset);
      ctx->gpr[0]                             = numberOfProcesses;

      // PL011_putc( UART0, '0' + firstFreePosition, true );

      // if(firstFreePosition != numberOfProcesses-1)
      //   scheduler(ctx);

      break;

    }

    case 0x05: { //0x05 => exec()]
      // PL011_putc( UART0, 'E', true );
      // PL011_putc( UART0, 'X', true );
      // PL011_putc( UART0, 'E', true );
      // PL011_putc( UART0, 'C', true );
      // PL011_putc( UART0, ' ', true );
      if(pcb[ executing ].status != STATUS_TERMINATED){
        pcb[ executing ].status = STATUS_EXECUTING;
        ctx->pc = (uint32_t) ctx->gpr[0];
        pcb[ executing ].basePriority = ctx->gpr[1];
        pcb[ executing ].currentPriority = 0;
      }

      // TODO de dat clear la stack

      break;

    }

    case 0x04: { //0x04 => exit()
      // TODO have to terminate current executing process (executing!!!)
      // PL011_putc( UART0, 'E', true );
      // PL011_putc( UART0, 'X', true );
      // PL011_putc( UART0, 'I', true );
      // PL011_putc( UART0, 'T', true );
      // PL011_putc( UART0, ' ', true );

      pcb[ executing ].status = STATUS_TERMINATED;
      scheduler(ctx);

      break;

    }

    case 0x06: { //0x06 => kill()
      // TODO have to terminate process with pid pidToTerminate
      uint32_t pidToTerminate = ctx->gpr[0];
      uint32_t signal         = ctx->gpr[1];
      if(pidToTerminate != 0){
        for(int i = 0; i < numberOfProcesses; i++)
          if(pcb[ i ].pid == pidToTerminate)
            pcb[ i ].status = STATUS_TERMINATED;
      }


      break;
    }

    case 0x10: { //0x10 => pipe()
      // ctx->gpr[0] = -1;
      int *filedes = ctx->gpr[0];
      filedes[0] = -1;
      filedes[1] = -1;
      int doneFirst = 0;
      for(int i = 0; i < 64; i++){
        if(pipes[i].isFree){
          if(!doneFirst){
            filedes[0] = i;
            pipes[i].isFree = 0;
            doneFirst = 1;
          }
          else{
            filedes[1] = i;
            pipes[i].isFree = 0;
            break;
          }
        }
      }

      break;
    }

    case 0x11: { //0x11 => readPipe()
      int fd = ctx->gpr[0];
      int erase = ctx->gpr[1];
        if((!pipes[fd].isClosed) && pipes[fd].isWritten == 1){
            ctx->gpr[0] = pipes[fd].message;
            if(erase == 1) pipes[fd].isWritten = 0;

        }
        else ctx->gpr[0] = UNWRITTEN;

      break;
    }

    case 0x12: { //0x12 => writePipe()
      // int pipeId = ctx->gpr[0];
      int fd = ctx->gpr[0];
      // printNumber(555555);
      if(!pipes[fd].isClosed && !pipes[fd].isWritten){
          pipes[fd].message = ctx->gpr[1];
          if(pipes[fd].message == UNWRITTEN) pipes[fd].isWritten = 0;
          else pipes[fd].isWritten = 1;
        }

      break;
    }

    default   : { // 0x?? => unknown/unsupported
      break;
    }
  }

  return;
}
