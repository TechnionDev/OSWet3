//
// Created by Gur Telem on 17/06/2021.
//

#ifndef OSWET3_GLOBAL_CONNECTIONS_COUNTER_H
#define OSWET3_GLOBAL_CONNECTIONS_COUNTER_H
#include "segel.h"

void initConnCounter(void);
int getConnCounter(void);
void incConnCounter(void);
void decConnCounter(void);

#endif //OSWET3_GLOBAL_CONNECTIONS_COUNTER_H
