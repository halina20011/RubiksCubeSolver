#include "mcode.h"

const static char *TAG = "mcode";

struct CommandQueue commandQueue = {};

extern struct Motor *motors;

// create a command queue
void commandQueueInit(){
    commandQueue.currCommandIndex = 0;
    commandQueue.lastCommandIndex = 0;
    commandQueue.size = 0;
    commandQueue.idCounter = 0;
}

uint8_t commandQueueSize(){
    return (uint8_t)commandQueue.size;
}

bool commandQueueFull(){
    return COMMAND_QUEUE_SIZE <= commandQueue.size;
}

uint8_t commandQueuePush(struct Command *command){
    if(COMMAND_QUEUE_SIZE <= commandQueue.size){
        return 1;
    }

    commandQueue.size++;
    
    command->id = commandQueue.idCounter++;
    memcpy(&commandQueue.queue[commandQueue.lastCommandIndex], command, sizeof(struct Command));

    commandQueue.lastCommandIndex = (commandQueue.lastCommandIndex + 1) % COMMAND_QUEUE_SIZE;

    return 0;
}

uint8_t commandQueuePop(struct Command *command){
    if(commandQueue.size == 0){
        return 1;
    }

    memcpy(command, &commandQueue.queue[commandQueue.currCommandIndex], sizeof(struct Command));
    commandQueue.currCommandIndex = (commandQueue.currCommandIndex + 1) % COMMAND_QUEUE_SIZE;

    commandQueue.size--;

    return 0;
}

void processCommandQueue(){
    if(commandQueue.size){
        struct Command command = {};
        ESP_LOGI(TAG, "poping");
        if(commandQueuePop(&command) != 0){
            ESP_LOGI(TAG, "empty");
            return;
        }

        const int cmdId = command.id;
        if(command.commandType == COMMAND_TYPE_R){
            const uint8_t id = command.motorId;
            const int deg = command.deg;
            const uint8_t direction = command.direction;

            char data[30] = {};
            snprintf(data, 30, "processing;%i\n", id);
            wsSend(command.fd, command.handle, data);

            // WS_SEND(command.req, 30, "processing;%i", cmdId);
            //
            motorRotateDeg(id, deg, direction);
            // WS_SEND(command.req, 30, "finished;%i", cmdId);
        }
        else{

        }
    }
}

uint8_t parseCommand(char *commandBuffer, struct Command *command){
    char commandTypeChar = commandBuffer[0];

    // ESP_LOGI(TAG, "buffer %s\n", commandBuffer);
    // ESP_LOGI(TAG, "commandType: %c %i\n", commandTypeChar, commandTypeChar == 'R');
    if(commandTypeChar == 'R'){
        int moveType = 0;
        int i1 = 0, i2 = 0, i3 = 0;
        sscanf(commandBuffer, "R%i %i %i %i", &moveType, &i1, &i2, &i3);
        // ESP_LOGI(TAG, "parsed %i %i %i", i1, i2, i3);
        command->commandType = COMMAND_TYPE_R;

        command->motorId = i1;
        command->deg = i2;
        command->direction = i3;
        
        return 0;
    }
    else if(commandTypeChar == 'C'){
        command->commandType = COMMAND_TYPE_C;
        return 0;
    }

    return 1;
}

uint8_t handleConnection(httpd_req_t *req, char *message){
    if(commandQueueFull()){
        char data[] = "commandQueueIsFull;-1\n";
        wsSendReq(req, data);
    }
    
    struct Command command = {};
    
    if(parseCommand(message, &command) == 0){
        command.handle = req->handle;
        command.fd = httpd_req_to_sockfd(req);
        int id = commandQueuePush(&command);
        char data[30] = {};
        snprintf(data, 30, "ok;%i\n", id);
        wsSendReq(req, data);
    }
    else{
        char data[] = "invalidCommand;-1\n";
        wsSendReq(req, data);
    }

    // char data[] = "UwU\n";
    // socketTransmit(sock, data);
    return 0;
}
