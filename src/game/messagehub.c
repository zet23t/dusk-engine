#include "messagehub.h"
#include <memory.h>
#include <raylib.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#define MESSAGE_TYPE(type) \
    int MessageId_##type() { return __COUNTER__; }
#include "messagetypelist.h"
#undef MESSAGE_TYPE

static int _messageCount;
static int _messageCapacity;
static int _messageIdCounter;
static int _messageIdOffset;
static int _frame;
static MessageHubMessage* _messages;

void MessageHub_init()
{
    _messageCapacity = 16;
    _messageCount = 0;
    // Message id 0 = subscribe
    _messageIdCounter = 1;
    _messageIdOffset = 1;
    _messages = (MessageHubMessage*)malloc(sizeof(MessageHubMessage) * _messageCapacity);
}

MessageHubMessage* MessageHub_getMessage(int* messageId)
{
    if (*messageId >= _messageIdCounter) {
        *messageId = _messageIdCounter;
        return NULL;
    }
    int index = *messageId - _messageIdOffset;
    if (index < 0) {
        *messageId = _messageIdCounter;
        return NULL;
    }
    *messageId += 1;
    return &_messages[index];
}

static MessageHubMessage* MessageHub_queue(int messageTypeId)
{
    if (_messageCount >= _messageCapacity) {
        _messageCapacity *= 2;
        _messages = (MessageHubMessage*)realloc(_messages, sizeof(MessageHubMessage) * _messageCapacity);
    }
    _messages[_messageCount] = (MessageHubMessage) {
        .messageTypeId = messageTypeId,
        .frame = _frame,
        .messageId = _messageIdCounter,
        .time = GetTime(),
    };
    _messageCount++;
    _messageIdCounter++;
    return &_messages[_messageCount - 1];
}

void MessageHub_process()
{
    _frame++;
    int trimIndex = 0;
    while (trimIndex < _messageCount && _messages[trimIndex++].frame < _frame - 2) {
        // nop
    }
    if (trimIndex == 0) {
        return;
    }
    if (trimIndex == _messageCount) {
        _messageIdOffset += _messageCount;
        _messageCount = 0;
        return;
    }
    for (int i= 0; i<_messageCount - trimIndex;i+=1)
    {
        _messages[i] = _messages[trimIndex + i];
    }
    _messageCount -= trimIndex;
    _messageIdOffset += trimIndex;
}

#define MESSAGE_TYPE(type) \
    void MessageHub_queue##type(type msg) { MessageHub_queue(MessageId_##type())->data##type = msg; }
#include "messagetypelist.h"
#undef MESSAGE_TYPE
