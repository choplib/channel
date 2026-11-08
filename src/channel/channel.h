#ifndef CGC_CHANNEL
#define CGC_CHANNEL

#include <stdlib.h>

typedef struct channel channel;

channel *channel_create(size_t elem_size, size_t cap);

int channel_send(channel *ch, const void *src);
int channel_recv(channel *ch, void *dst);

void channel_destroy(channel *ch);

#endif
