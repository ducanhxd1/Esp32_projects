#ifndef _DATA_H_
#define _DATA_H_

typedef struct struct_message
{ 
    float temp;
    float humi;
    uint16_t smokeValueA;
    uint16_t smokeValueB;
    uint8_t slaveID;
} struct_message;
    

#endif