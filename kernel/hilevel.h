/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#ifndef __HILEVEL_H
#define __HILEVEL_H

// Include functionality relating to newlib (the standard C library).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// Include functionality relating to the   kernel.

#include "GIC.h"
#include "PL011.h"
#include "SP804.h"
#include "PL050.h"
#include "PL111.h"
#include   "SYS.h"

#include "lolevel.h"
#include "int.h"
#include "scheduling.h"
#include "defines.h"
#include "letters.h"

typedef int pid_t;

typedef enum {
  STATUS_CREATED,
  STATUS_READY,
  STATUS_EXECUTING,
  STATUS_WAITING,
  STATUS_TERMINATED
} status_t;

typedef struct {
  uint32_t cpsr, pc, gpr[ 13 ], sp, lr;
} ctx_t;

typedef struct pcb_t {
     pid_t    pid;
  status_t status;
     ctx_t    ctx;
     uint32_t tos;
     uint32_t basePriority;
     uint32_t currentPriority;
     uint32_t currentTime;
} pcb_t;

typedef struct pipe_t{
  /*pid_t process1, process2;
  int data[2];*/
  int message;
  int isWritten;
  int isClosed;
  int isFree;
} pipe_t;

typedef struct drawingPosition{
  int x;
  int y;
} drawingPosition;

#endif
