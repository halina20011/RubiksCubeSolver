#ifndef __MCODE__
#define __MCODE__

#include <freertos/FreeRTOS.h>

#include "wifi.h"
#include "motor.h"

#define COMMAND_QUEUE_SIZE 50
#define MAX_COMMAND_SIZE 25

// when every command is received parse it,
// if its valid command send ok with command queue id ok;[id]
// if not send someting like invalid command

// commands

// rotate motor [motor]
// R0 [motor] [deg] [direction]

// rotate [motor] [count] times 90 degres to the [direction] 
// R9 [motor] [count] [direction]

// captures curr color
// C0

// set color integration time
// I [integration time]

enum CommandType{
    COMMAND_TYPE_R,
    COMMAND_TYPE_C,
    COMMAND_TYPE_MOVE,
    COMMAND_TYPE_NULL,
};

#define MS2(s1 ,s2) (MAX(sizeof(s1), sizeof(s2)))
#define MAX_COMMAND_DATA_SIZE (MS2(struct CommandRotate));

struct Command{
    enum CommandType commandType;
    int id;

    httpd_handle_t handle;
    int fd;
    
    uint8_t motorId, direction;
    int deg;
};

struct CommandQueue{
    struct Command queue[COMMAND_QUEUE_SIZE];
    int currCommandIndex, lastCommandIndex;
    int size;
    int idCounter;
};

void commandQueueInit(void);
void processCommandQueue(void);

uint8_t mcodeQueueSize(void);

uint8_t commandQueueSize(void);
bool commandQueueFull(void);
uint8_t commandQueuePush(struct Command *command);
uint8_t commandQueuePop(struct Command *command);

uint8_t parseCommand(char *commandBuffer, struct Command *command);

uint8_t handleConnection(httpd_req_t *req, char *message);

#endif
