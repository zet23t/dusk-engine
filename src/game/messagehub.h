#ifndef __MESSAGEHUB_H__
#define __MESSAGEHUB_H__

#define DECLARATION
#include "messagetypelist.h"
#undef DECLARATION

typedef struct MessageHubMessage {
    int messageTypeId;
    int messageId;
    float time;
    int frame;
    union {
#define MESSAGE_TYPE(t) t data##t;
#include "messagetypelist.h"
#undef MESSAGE_TYPE
    };
} MessageHubMessage;

void MessageHub_init();

#define MESSAGE_TYPE(type) void MessageHub_queue##type(type msg);
#include "messagetypelist.h"
#undef MESSAGE_TYPE

void MessageHub_process();
// Returns NULL if no message is available
MessageHubMessage* MessageHub_getMessage(int *messageId);

#define MESSAGE_TYPE(type) int MessageId_##type();
#include "messagetypelist.h"
#undef MESSAGE_TYPE

#endif