#ifndef _SMOKE_SENSOR_H_
#define _SMOKE_SENSOR_H_

#include "BM22S2021-1.h"
#include "data.h"

void initSmokeSensor();
void printDataSmokeSensor(struct_message& data);

#endif  