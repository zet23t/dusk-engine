#include "messagehub.h"
#include <stdlib.h>
#include <memory.h>

#define MESSAGE_TYPE(type) int MessageId_#type() { return __COUNTER__; }
#include "messagetypelist.h"
#undef MESSAGE_TYPE

static MessageHubMessage* _messages;
static int _messageCount;
static int _messageCapacity;
static int _messageIdCounter;
static int _messageIdOffset;

void MessageHub_init()
{
    _messageCapacity = 16;
    _messageCount = 0;
    _messages = (MessageHubMessage*)malloc(sizeof(MessageHubMessage) * _messageCapacity);
}

MessageHubMessage* MessageHub_getMessage(int *messageId)
{
    if (*messageId >= _messageIdCounter)
    {
        *messageId = _messageIdCounter;
        return NULL;
    }
    int index = *messageId - _messageIdOffset;
    if (index < 0)
    {
        *messageId = _messageIdOffset;
        index = 0;
    }
}
