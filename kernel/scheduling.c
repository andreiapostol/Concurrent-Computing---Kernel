#include "scheduling.h"

// int getMaximumPriorityIndex( pcb_t* pcb, int size ){
//   int maxIndex = 0;
//   for (int i = 1; i < size; i++){
//      if(pcb[i].currentPriority > pcb[maxIndex].currentPriority
//        && pcb[i].status != STATUS_TERMINATED)
//        maxIndex = i;
//   }
//   return maxIndex;
// }
//
// void prioritize( pcb_t* pcb, int size ){
//   if(size == 1) return;
//   int maxIndex = getMaximumPriorityIndex(pcb, size);
//   for (int i = 0; i < size; i++){
//     if(i != maxIndex){
//       pcb[i].currentPriority += (pcb[i].basePriority / 10);
//       if(pcb[i].currentPriority > pcb[maxIndex].currentPriority)
//         pcb[maxIndex].currentPriority = 0;
//     }
//   }
// }
