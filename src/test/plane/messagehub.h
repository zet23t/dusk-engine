#ifndef __MESSAGEHUB_H__
#define __MESSAGEHUB_H__

typedef struct MessageHubMessage {
    int messageTypeId;
    void* message;
    int messageSize;
} MessageHubMessage;

void MessageHub_init();
void MessageHub_queue(int messageTypeId, void* message, int messageSize);
void MessageHub_process();
MessageHubMessage* MessageHub_getMessage(int *messageId);

#define MESSAGE_TYPE(type) int MessageId_#type();
#include "messagetypelist.h"
#undef MESSAGE_TYPE

#endif