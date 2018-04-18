#include "philo.h"

#define PHILO_NUMBER 19
int fildes[PHILO_NUMBER][2];

unsigned short lfsr2 = 0xACE1u;
unsigned bit2;

unsigned random2(){
  bit2  = ((lfsr2 >> 0) ^ (lfsr2 >> 2) ^ (lfsr2 >> 3) ^ (lfsr2 >> 5) ) & 1;
  return lfsr2 =  (lfsr2 >> 1) | (bit2 << 15);
}

void main_philo() {
  for(int i = 0; i < PHILO_NUMBER; i++){
    pipe(fildes[i]);
    switch(fork()){
      case -1:{
        break;
      }
      case 0:{
        philosopher(fildes[i]);
      }
      default:{
        if(i == PHILO_NUMBER - 1) waiter();
        break;
      }
    }
  }
}

void think(){   // should only be called once
  int r = (int)random2() % 100000;
  for(int i = 0; i < r; i++);

}
void pickUpLeftFork(int channel[2]){
  properWritePipe(channel[1], REQUESTLEFTFORK);
  printString("PHILOSOPHER ", 12);
  printNumber((channel[0] / 2));
  printString(" WAITS TO PICK LEFT FORK.", 25);
  printString("\n", 1);
  int flag = properReadPipe(channel[0]);
  if (flag == CANPICKUPFORK) {
    printString("PHILOSOPHER ", 12);
    printNumber((channel[0] / 2));
    printString(" PICKED LEFT FORK.", 18);
    printString("\n", 1);
  }
}

void pickUpRightFork(int channel[2]){
  properWritePipe(channel[1], REQUESTRIGHTFORK);
  printString("PHILOSOPHER ", 12);
  printNumber((channel[0] / 2));
  printString(" WAITS TO PICK RIGHT FORK.", 26);
  printString("\n", 1);
  int flag = properReadPipe(channel[0]);
  if (flag == CANPICKUPFORK) {
    printString("PHILOSOPHER ", 12);
    printNumber((channel[0] / 2));
    printString(" PICKED RIGHT FORK.", 19);
    printString("\n", 1);
  }
}

void eat(int channel[2]){
  printString("PHILOSOPHER ", 12);
  printNumber((channel[0] / 2));
  printString(" EATS.", 5);
  printString("\n", 1);
  properWritePipe(channel[1], HASEATEN);
}

void waiter(){
  int reservedForks[PHILO_NUMBER];
  int requestedLeft[PHILO_NUMBER];
  int requestedRight[PHILO_NUMBER];
  for(int i = 0; i < PHILO_NUMBER; i++){
    reservedForks[i] = FORKDOWN;
  }
  while(1){

    for(int i = 0; i < PHILO_NUMBER; i++){
      int requested = readPipe(fildes[i][1], 0);

      if(requested == REQUESTLEFTFORK){
        if(reservedForks[i] == FORKDOWN && reservedForks[(i+1) % PHILO_NUMBER] == FORKDOWN){
          requested = readPipe(fildes[i][1], 1);
          properWritePipe(fildes[i][0], CANPICKUPFORK);
          printString("ALOCAT FURCULITELE FILOZOFULUI ", 31);
          printNumber(i);
          printString("\n",1);
          reservedForks[i] = i;
          reservedForks[(i + 1 + PHILO_NUMBER) % PHILO_NUMBER] = i;
        }
      }

      if(requested == REQUESTRIGHTFORK){
        if(reservedForks[(i+1) % PHILO_NUMBER] == i){
          requested = readPipe(fildes[i][1], 1);
          printString("DAT FURCULITA DREAPTA FILOZOFULUI ", 34);
          printNumber(i);
          printString("\n", 1);
          properWritePipe(fildes[i][0], CANPICKUPFORK);
        }
      }

      if(requested == HASEATEN){
        requested = readPipe(fildes[i][1], 1);
        if(reservedForks[i] == i)reservedForks[i] = FORKDOWN;
        if(reservedForks[(i + 1 + PHILO_NUMBER) % PHILO_NUMBER] == i)
          reservedForks[(i + 1 + PHILO_NUMBER) % PHILO_NUMBER] = FORKDOWN;
      }


    }
  }
  exit(EXIT_SUCCESS);
}

void philosopher(int channel[2]){
  // while(1){
  //   properWritePipe(channel[0], 10);
  // }
  while(1){
    think();
    pickUpLeftFork(channel);
    pickUpRightFork(channel);
    eat(channel);
    exit(EXIT_SUCCESS);
  }
  exit(EXIT_SUCCESS);
}
