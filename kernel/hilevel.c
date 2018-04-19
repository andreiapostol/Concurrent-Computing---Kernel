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
uint16_t fb[ 600 ][ 800 ];
int consoleTime = 0;


unsigned short lfsr = 0xACE1u;
unsigned bit;

drawingPosition currentDashboardPosition = (drawingPosition)  {1, 1};
drawingPosition currentCommandsPosition  = (drawingPosition)  {1, 410};

int currentButtonSelection = 0;

void drawContiuously(char letter[12][9], int colour, drawingPosition *position){
  for(int i = 0; i < 12; i++)
    for(int j = 0; j < 9; j++){
      if(letter[i][j] == 1) fb[position->x + i][position->y + j] = colour;
    }
  position->y += 11;
}

void drawSpace(drawingPosition *position){
  position->y += 11;
}

void drawSpaces(int length, drawingPosition *position){
  for(int i = 0; i < length; i++) drawSpace(position);
}

void drawHorizontalLine(int y1, int y2, int x, int colour){
  for(int j = y1; j <= y2; j++)
    fb[x][j] = colour;
}

void colourBlock(drawingPosition leftUp, int length,
                int width, int colour){
   for(int i = leftUp.x; i <= leftUp.x + width; i++){
     for(int j = leftUp.y; j <= leftUp.y + length; j++)
     fb[i][j] = colour;
   }
}
unsigned random(){
  bit  = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
  return lfsr =  (lfsr >> 1) | (bit << 15);
}

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

    uint32_t id = GICC0->IAR;



    // printString("    ⬤       ⬤       ⬤       ⬤  \n", 44);
    // printString("  ╔══════════════════════════════╗  \n", 101);
    // printString("  ║░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒║  \n", 103);
    // printString("  ║▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░║  \n", 101);
    // printString("⬤ ║░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒║⬤  \n", 106);
    // printString("  ║▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░║  \n", 101);
    // printString("  ║░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒║  \n", 101);
    // printString("⬤ ║▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░║⬤  \n", 106);
    // printString("  ║░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒║  \n", 101);
    // printString("  ║▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░║  \n", 101);
    // printString("⬤ ║░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒║⬤  \n", 106);
    // printString("  ║▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░║  \n", 101);
    // printString("  ║░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒║  \n", 101);
    // printString("⬤ ║▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░║⬤  \n", 106);
    // printString("  ║░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒║  \n", 101);
    // printString("  ║▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░▒░║  \n", 101);
    // printString("  ╚══════════════════════════════╝  \n", 101);
    // printString("    ⬤       ⬤       ⬤       ⬤  \n", 44);

}

void reverseString(char *s, int length)
{
    int c, i, j;

    for (i = 0, j = length - 1; i < j; i++, j--)
    {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void displayString(char *str, int length, int colour, drawingPosition *position){
   for(int i = 0; i < length; i++){
     if(str[i] == 'a' || str[i] == 'A') drawContiuously(letterA, colour, position);
     else if (str[i] == 'c' || str[i] == 'C') drawContiuously(letterC, colour, position);
     else if (str[i] == 'd' || str[i] == 'D') drawContiuously(letterD, colour, position);
     else if (str[i] == 'e' || str[i] == 'E') drawContiuously(letterE, colour, position);
     else if (str[i] == 'h' || str[i] == 'H') drawContiuously(letterH, colour, position);
     else if (str[i] == 'i' || str[i] == 'I') drawContiuously(letterI, colour, position);
     else if (str[i] == 'l' || str[i] == 'L') drawContiuously(letterL, colour, position);
     else if (str[i] == 'm' || str[i] == 'M') drawContiuously(letterM, colour, position);
     else if (str[i] == 'n' || str[i] == 'N') drawContiuously(letterN, colour, position);
     else if (str[i] == 'o' || str[i] == 'O') drawContiuously(letterO, colour, position);
     else if (str[i] == 'p' || str[i] == 'P') drawContiuously(letterP, colour, position);
     else if (str[i] == 'r' || str[i] == 'R') drawContiuously(letterR, colour, position);
     else if (str[i] == 's' || str[i] == 'S') drawContiuously(letterS, colour, position);
     else if (str[i] == 't' || str[i] == 'T') drawContiuously(letterT, colour, position);
     else if (str[i] == 'u' || str[i] == 'U') drawContiuously(letterU, colour, position);
     else if (str[i] == 'x' || str[i] == 'X') drawContiuously(letterX, colour, position);
     else if (str[i] == 'y' || str[i] == 'Y') drawContiuously(letterY, colour, position);
     else if (str[i] == '0') drawContiuously(number0, colour, position);
     else if (str[i] == '1') drawContiuously(number1, colour, position);
     else if (str[i] == '2') drawContiuously(number2, colour, position);
     else if (str[i] == '3') drawContiuously(number3, colour, position);
     else if (str[i] == '4') drawContiuously(number4, colour, position);
     else if (str[i] == '5') drawContiuously(number5, colour, position);
     else if (str[i] == '6') drawContiuously(number6, colour, position);
     else if (str[i] == '7') drawContiuously(number7, colour, position);
     else if (str[i] == '8') drawContiuously(number8, colour, position);
     else if (str[i] == '9') drawContiuously(number9, colour, position);
     else if (str[i] == ' ') drawSpace(position);


   }
}
void displayNumber(int x, int colour, drawingPosition *position){
  if(x == 0) {drawContiuously(number0, colour, position); return;};

  int i = -1;
  char *str;
  while(x){
    str[++i] = '0' + (x % 10);
    // write(STDOUT_FILENO, str, 1);
    x /= 10;
  }
  reverseString(str, i+1);
  displayString(str, i+1, colour, position);

}
void renderDashboardDisplay(){

  drawingPosition start = {10, 10};

  for(int i = start.x; i < start.x + (numberOfProcesses + 1) * 15; i++)
    for(int j = start.y; j < start.y + 300; j++)
    fb[i][j] = BACKCOLOUR;
  currentDashboardPosition = start;
  displayString("PID", 3, BLACK, &currentDashboardPosition);
  drawSpaces(3, &currentDashboardPosition);
  displayString("TIME", 4, BLACK, &currentDashboardPosition);
  drawSpaces(3, &currentDashboardPosition);
  displayString("STATUS", 6, BLACK, &currentDashboardPosition);
  drawSpaces(3, &currentDashboardPosition);
  displayString("PRIORITY", 8, BLACK, &currentDashboardPosition);


  for(int i = 0; i < numberOfProcesses; i++){

    currentDashboardPosition.x = start.x + 14 * (i+1);
    currentDashboardPosition.y = start.y;
    displayNumber(pcb[i].pid, BLACK, &currentDashboardPosition);
    if(pcb[i].pid < 10) drawSpaces(5, &currentDashboardPosition);
    else drawSpaces(4, &currentDashboardPosition);

    displayNumber(pcb[i].currentTime, BLACK, &currentDashboardPosition);
    if(pcb[i].currentTime < 10) drawSpaces(6, &currentDashboardPosition);
    else if(pcb[i].currentTime < 100) drawSpaces(5, &currentDashboardPosition);
    else if(pcb[i].currentTime < 1000) drawSpaces(4, &currentDashboardPosition);
    else if(pcb[i].currentTime < 10000) drawSpaces(3, &currentDashboardPosition);
    else drawSpaces(2, &currentDashboardPosition);

    if(pcb[i].status == STATUS_READY) {
      displayString("READY", 5, GREEN, &currentDashboardPosition);
      drawSpaces(4, &currentDashboardPosition);
    }
    else if (pcb[i].status == STATUS_EXECUTING) {
      displayString("EXEC", 4, BLACK, &currentDashboardPosition);
      drawSpaces(5, &currentDashboardPosition);
    }
    else if (pcb[i].status == STATUS_TERMINATED) {
      displayString("TERM", 4, RED, &currentDashboardPosition);
      drawSpaces(5, &currentDashboardPosition);
    }
    else drawSpaces(9, &currentDashboardPosition);

    displayNumber(pcb[i].currentPriority, WHITE, &currentDashboardPosition);


  }

  // for(int i = 0; i < numberOfProcesses; i++)
}

void drawBorder(drawingPosition button, int length, int width, int colour){
  colourBlock((drawingPosition) {button.x - 3,         button.y - 3     }, length + 6, 3    , colour);
  colourBlock((drawingPosition) {button.x + width,     button.y - 3     }, length + 6, 3    , colour);
  colourBlock((drawingPosition) {button.x - 3,         button.y - 3     }, 3,          width + 6, colour);
  colourBlock((drawingPosition) {button.x - 3,         button.y + length}, 3,          width + 6, colour);
}

void renderCommands(){

  drawingPosition execP3Button = {50, 430}, execP4Button = {50, 550}, execP5Button = {50, 670};
  drawingPosition philoButton = {execP4Button.x + 70, 530}, terminateButton = {philoButton.x + 70, 530};

  colourBlock(execP3Button, 100, 35, BLACK);
  currentCommandsPosition.x = execP3Button.x + 12;
  currentCommandsPosition.y = execP3Button.y + 12;
  displayString("EXEC P3", 7, WHITE, &currentCommandsPosition);

  colourBlock(execP4Button, 100, 35, BLACK);
  currentCommandsPosition.x = execP4Button.x + 12;
  currentCommandsPosition.y = execP4Button.y + 12;
  displayString("EXEC P4", 7, WHITE, &currentCommandsPosition);

  colourBlock(execP5Button, 100, 35, BLACK);
  currentCommandsPosition.x = execP5Button.x + 12;
  currentCommandsPosition.y = execP5Button.y + 12;
  displayString("EXEC P5", 7, WHITE, &currentCommandsPosition);

  colourBlock(philoButton, 140, 35, BLACK);
  currentCommandsPosition.x = philoButton.x + 12;
  currentCommandsPosition.y = philoButton.y + 12;
  displayString("EXEC PHILO", 10, WHITE, &currentCommandsPosition);

  colourBlock(terminateButton, 140, 35, RED);
  currentCommandsPosition.x = terminateButton.x + 12;
  currentCommandsPosition.y = terminateButton.y + 16;
  displayString("TERMINATE",  9, WHITE, &currentCommandsPosition);

  drawingPosition currentButton;
  switch(currentButtonSelection){
    case 0: { currentButton = execP3Button; break; };
    case 1: { currentButton = execP4Button; break; };
    case 2: { currentButton = execP5Button; break; };
    case 3: { currentButton = philoButton; break; };
    case 4: { currentButton = terminateButton; break; };
    default : currentButton = execP5Button;
  }

  if(currentButtonSelection == 0) drawBorder(execP3Button, 100, 35, WHITE);
  else drawBorder(execP3Button, 100, 35, SKY_BLUE);
  if(currentButtonSelection == 1) drawBorder(execP4Button, 100, 35, WHITE);
  else drawBorder(execP4Button, 100, 35, SKY_BLUE);
  if(currentButtonSelection == 2) drawBorder(execP5Button, 100, 35, WHITE);
  else drawBorder(execP5Button, 100, 35, SKY_BLUE);
  if(currentButtonSelection == 3) drawBorder(philoButton, 140, 35, WHITE);
  else drawBorder(philoButton, 140, 35, SKY_BLUE);
  if(currentButtonSelection == 4) drawBorder(terminateButton, 140, 35, WHITE);
  else drawBorder(terminateButton, 140, 35, SKY_BLUE);

  colourBlock((drawingPosition) {300, 450}, 20, 30, RED);
  currentCommandsPosition.x = 305;
  currentCommandsPosition.y = 455;
  displayNumber(currentButtonSelection, BLUE, &currentCommandsPosition);




}


 void resetAllPriorities(){
   for(int i = 0; i < numberOfProcesses; i++)
    pcb[i].currentPriority /= 2;
 }

 int getMaximumPriorityIndex(){
   int maxIndex = 0;
   for (int i = 1; i < numberOfProcesses; i++){
      if(pcb[i].currentPriority > pcb[maxIndex].currentPriority
        && pcb[i].status != STATUS_TERMINATED)
        maxIndex = i;
   }
  //  printNumber(maxIndex);
   return maxIndex;
 }

 void prioritize(){
   if(numberOfProcesses == 1) return;
   int maxIndex = getMaximumPriorityIndex();
   if(pcb[maxIndex].currentPriority > 250) resetAllPriorities();
   for (int i = 0; i < numberOfProcesses; i++){
     if(i != maxIndex && pcb[i].status == STATUS_READY){
       pcb[i].currentPriority += (pcb[i].basePriority / 10) + 10;
       if(pcb[i].currentPriority >= pcb[maxIndex].currentPriority)
         pcb[maxIndex].currentPriority = 0;
     }
   }
 }


void scheduler( ctx_t* ctx ) {

    // if(numberOfProcesses != 1){
    //   int exExecuting = executing;
    //   do{
    //     nextProcess = (++exExecuting) % numberOfProcesses;
    //   }
    //   while(pcb[ nextProcess ].status == STATUS_TERMINATED);
    // }
    // else{
    //   nextProcess = executing;
    // }

    consoleTime = (consoleTime + 1) % 4;
    if(!consoleTime){
      prioritize(numberOfProcesses);
      nextProcess = getMaximumPriorityIndex(numberOfProcesses);
    }
    else nextProcess = 0;

    memcpy( &pcb[ executing ].ctx, ctx, sizeof( ctx_t ) );        // preserve P_1

    if(pcb[ executing ].status != STATUS_TERMINATED)
      pcb[ executing ].status = STATUS_READY;                     // update   P_1 status

    memcpy( ctx, &pcb[ nextProcess ].ctx, sizeof( ctx_t ) );      // restore  P_2
    pcb[ nextProcess ].status = STATUS_EXECUTING;                 // update   P_2 status
    executing = nextProcess;                                      // update   index => P_2

    return;
}

extern void main_console();
// extern uint32_t tos_console;
extern uint32_t tos_general;


//
void hilevel_handler_rst( ctx_t* ctx              ) {

    TIMER0->Timer1Load  = 0x00020000; // select period = 2^20 ticks ~= 1 sec
    TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
    TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
    TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
    TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

    GICC0->PMR          = 0x000000F0; // unmask all            interrupts
    GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
    GICC0->CTLR         = 0x00000001; // enable GIC interface
    GICD0->CTLR         = 0x00000001; // enable GIC distributor

    SYSCONF->CLCD      = 0x2CAC;     // per per Table 4.3 of datasheet
    LCD->LCDTiming0    = 0x1313A4C4; // per per Table 4.3 of datasheet
    LCD->LCDTiming1    = 0x0505F657; // per per Table 4.3 of datasheet
    LCD->LCDTiming2    = 0x071F1800; // per per Table 4.3 of datasheet

    LCD->LCDUPBASE     = ( uint32_t )( &fb );

    LCD->LCDControl    = 0x00000020; // select TFT   display type
    LCD->LCDControl   |= 0x00000008; // select 16BPP display mode
    LCD->LCDControl   |= 0x00000800; // power-on LCD controller
    LCD->LCDControl   |= 0x00000001; // enable   LCD controller

    PS20->CR           = 0x00000010; // enable PS/2    (Rx) interrupt
    PS20->CR          |= 0x00000004; // enable PS/2 (Tx+Rx)
    PS21->CR           = 0x00000010; // enable PS/2    (Rx) interrupt
    PS21->CR          |= 0x00000004; // enable PS/2 (Tx+Rx)

    uint8_t ack;

          PL050_putc( PS20, 0xF4 );  // transmit PS/2 enable command
    ack = PL050_getc( PS20       );  // receive  PS/2 acknowledgement
          PL050_putc( PS21, 0xF4 );  // transmit PS/2 enable command
    ack = PL050_getc( PS21       );  // receive  PS/2 acknowledgement

    GICC0->PMR         = 0x000000F0; // unmask all          interrupts
    GICD0->ISENABLER1 |= 0x00300000; // enable PS2          interrupts
    GICC0->CTLR        = 0x00000001; // enable GIC interface
    GICD0->CTLR        = 0x00000001; // enable GIC distributor

    int_enable_irq();


    memset( &pcb[ 0 ], 0, sizeof( pcb_t ) );
    pcb[ 0 ].pid      = 0;
    pcb[ 0 ].status   = STATUS_READY;
    pcb[ 0 ].ctx.cpsr = 0x50;
    pcb[ 0 ].ctx.pc   = ( uint32_t )( &main_console );
    pcb[ 0 ].ctx.sp   = ( uint32_t )( &(tos_general)  );
    pcb[ 0 ].tos   = ( uint32_t )( &(tos_general)  );
    pcb[ 0 ].basePriority = 0;
    pcb[ 0 ].currentPriority = 50;

    for(int i = 1; i < 64; i++){
      pcb[i].status = STATUS_TERMINATED;
    }

    memcpy( ctx, &pcb[ 0 ].ctx, sizeof( ctx_t ) );


    pcb[ 0 ].status = STATUS_EXECUTING;
    executing = 0;
    for(int i = 0; i < 64; i++){
      pipes[i].isWritten = 0;
      pipes[i].isClosed  = 0;
      pipes[i].isFree    = 1;
      pipes[i].message   = UNWRITTEN;
    }
// (8, 27, 56)
// 08 1B 38
// 0 11100 11011 01000
    for(int i = 0; i < 600; i++)
      for(int j = 0; j < 800; j++)
      fb[i][j] = SKY_BLUE;
    colourBlock((drawingPosition) {0, 398}, 4, 799, BLACK);
    colourBlock((drawingPosition) {300, 398}, 399, 4, BLACK);



    // for(int i = 48; i <= 49; i++)
    //   for(int j = 348; j <= 539; j++)
    //   fb[i][j] = WHITE;
    // drawHorizontalLine(345, 542, 46, BLACK);
    // drawHorizontalLine(348, 539, 47, BLACK);
    // drawHorizontalLine(348, 539, 48, BLACK);

    // colourBlock((drawingPosition) {44, 335}, 200, 3, BLACK);

    renderDashboardDisplay();
    // colourBlock((){100, 200}, {100, 500}, {400, 500}, {400, 200}, RED);
    return;
}
// Q = 10 90
// P = 19 99
// A = 1E 9E
// L = 26 A6
// Z = 2C AC
// M = 32 B2

// E0 4B E0 CB <-
// E0 4D E0 CD
// E0 48 E0 C8 ^
// E0 50 E0 D0
// 1C 9C ENTER
void hilevel_handler_irq(ctx_t* ctx) {
  // Step 2: read  the interrupt identifier so we know the source.

  uint32_t id = GICC0->IAR;

  if( id == GIC_SOURCE_PS20 ) {
    uint8_t x = PL050_getc( PS20 );

    switch(x){
      case 0x4B: { // LEFT
        if(currentButtonSelection <= 2)
          currentButtonSelection = (currentButtonSelection + 3 - 1) % 3;
        break;
      }
      case 0x4D: { // RIGHT
        if(currentButtonSelection <= 2)
          currentButtonSelection = (currentButtonSelection + 3 + 1) % 3;
        break;
      }

      case 0x48: { //SUS
        if(currentButtonSelection <= 2) currentButtonSelection = 4;
        else if(currentButtonSelection == 3) currentButtonSelection = 1;
        else if(currentButtonSelection == 4) currentButtonSelection = 3;


        break;
      }

      case 0x50: { //JOS
        if(currentButtonSelection <= 2) currentButtonSelection = 3;
        else if(currentButtonSelection == 3) currentButtonSelection = 4;
        else if(currentButtonSelection == 4) currentButtonSelection = 1;
        break;
      }

      case 0x1C: {
        if(currentButtonSelection == 0){
          // fork();
          // exec(load("P3"), 100);
        }
        break;
      }
    }
    // switch(x){
    //   case 0x19:{
    //     drawContiuously(letterP, BLACK);
    //     break;
    //   }
    //   case 0x17: {
    //     drawContiuously(letterI, BLACK);
    //     break;
    //   }
    //   case 0x20: {
    //     drawContiuously(letterD, BLACK);
    //     break;
    //   }
    //   case 0x14: {
    //     drawContiuously(letterT, BLACK);
    //     break;
    //   }
    //   case 0x12: {
    //     drawContiuously(letterE, BLACK);
    //     break;
    //   }
    //   case 0x32: {
    //     drawContiuously(letterM, BLACK);
    //     break;
    //   }
    //   case 0x18: {
    //     drawContiuously(letterO, BLACK);
    //     break;
    //   }
    //   case 0x2E: {
    //     drawContiuously(letterC, BLACK);
    //     break;
    //   }
    //   case 0x13: {
    //     drawContiuously(letterR, BLACK);
    //     break;
    //   }
    // }

    PL011_putc( UART0, '0',                      true );
    PL011_putc( UART0, '<',                      true );
    PL011_putc( UART0, itox( ( x >> 4 ) & 0xF ), true );
    PL011_putc( UART0, itox( ( x >> 0 ) & 0xF ), true );
    PL011_putc( UART0, '>',                      true );
  }
  else if( id == GIC_SOURCE_PS21 ) {
    uint8_t x = PL050_getc( PS21 );

    PL011_putc( UART0, '1',                      true );
    PL011_putc( UART0, '<',                      true );
    PL011_putc( UART0, itox( ( x >> 4 ) & 0xF ), true );
    PL011_putc( UART0, itox( ( x >> 0 ) & 0xF ), true );
    PL011_putc( UART0, '>',                      true );
  }

  if( id == GIC_SOURCE_TIMER0 ) {
    TIMER0->Timer1IntClr = 0x01;
    pcb[executing].currentTime++;
    scheduler(ctx);
  }

  GICC0->EOIR = id;
  // customWrite("\e[1;1H\e[2J", 12);
  // renderScreen();
  renderDashboardDisplay();
  renderCommands();

  return;
}

void hilevel_handler_svc( ctx_t* ctx, uint32_t id ) {

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
      break;
    }

    case 0x03: { // 0x03 => fork()


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
      uint32_t offset_tos = (uint32_t) pcb[ executing ].tos - (uint32_t) ctx->sp;

      pcb[ firstFreePosition ].tos          =  newTos;
      pcb[ firstFreePosition ].ctx.sp       =  pcb[ firstFreePosition ].tos + offset_tos;

      memcpy((void*) pcb[ firstFreePosition ].tos, (void*) pcb[ executing ].tos , offset);
      ctx->gpr[0]                             = numberOfProcesses;

      break;

    }

    case 0x05: { //0x05 => exec()]

      if(pcb[ executing ].status != STATUS_TERMINATED){
        pcb[ executing ].status = STATUS_EXECUTING;
        ctx->pc = (uint32_t) ctx->gpr[0];
        pcb[ executing ].basePriority = ctx->gpr[1];
        pcb[ executing ].currentPriority = 100;
      }

      // TODO de dat clear la stack

      break;

    }

    case 0x04: { //0x04 => exit()
      pcb[ executing ].status = STATUS_TERMINATED;
      scheduler(ctx);

      break;

    }

    case 0x06: { //0x06 => kill()
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
