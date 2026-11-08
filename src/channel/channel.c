#include <pthread.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

// FEATURE: zamani gelince runtime yonetimi icin
// queue objeleri eklendi ama kullanilmiyor
/*
typedef struct queue queue;

struct queue {
  queue *head;
};

typedef struct queue_node queue_node;
struct queue_node {
  queue_node *next;
};
*/

// TODO: pointer lari yerine nesnelerin kendilerinin tutulmasi icin
// size degiskeni acik birakildi
typedef struct {
    unsigned char *data;

    size_t elem_size;
    size_t cap;
    size_t len;
    size_t head;
    size_t tail;

    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} channel;

channel *channel_create(size_t size, size_t cap) {
    if (size == 0 || cap == 0) {
        return NULL;
    }

    if (cap > (SIZE_MAX - sizeof(channel)) / size) {
        return NULL;
    }

    channel *ch = malloc(sizeof *ch + cap * size);
    if (ch == NULL) {
        return NULL;
    }

    memset(ch, 0, sizeof *ch);

    ch->data = (unsigned char *)(ch + 1);
    ch->elem_size = size;
    ch->cap = cap;

    if (pthread_mutex_init(&ch->mutex, NULL) != 0) {
        free(ch);
        return NULL;
    }

    if (pthread_cond_init(&ch->not_empty, NULL) != 0) {
        pthread_mutex_destroy(&ch->mutex);
        free(ch);
        return NULL;
    }

    if (pthread_cond_init(&ch->not_full, NULL) != 0) {
        pthread_cond_destroy(&ch->not_empty);
        pthread_mutex_destroy(&ch->mutex);
        free(ch);
        return NULL;
    }

    return ch;
}

/*
channel *make_channel(int cap, int size) {
  channel *ch;

  if (cap <= 0) {
    return NULL;
  }

  ch = malloc(sizeof *ch + (size_t)cap * sizeof(void *));
  if (ch == NULL) {
    return NULL;
  }

  memset(ch, 0, sizeof *ch);

  ch->data = (void *)(ch + 1);
  ch->cap = cap;
  ch->size = size;
  ch->len = 0;

  pthread_mutex_init(&ch->mutex, NULL);
  pthread_cond_init(&ch->not_empty, NULL);
  pthread_cond_init(&ch->not_full, NULL);

  return ch;
}
*/

int channel_recv(channel *ch, void *dst) {
    if (ch == NULL || dst == NULL) {
        return -1;
    }

    pthread_mutex_lock(&ch->mutex);

    while (ch->len == 0) {
        pthread_cond_wait(&ch->not_empty, &ch->mutex);
    }

    memcpy(
        dst,
        ch->data + ch->head * ch->elem_size,
        ch->elem_size
    );

    ch->head = (ch->head + 1) % ch->cap;
    ch->len--;

    pthread_cond_signal(&ch->not_full);
    pthread_mutex_unlock(&ch->mutex);

    return 0;
}


int channel_send(channel *ch, const void *src) {
    if (ch == NULL || src == NULL) {
        return -1;
    }

    pthread_mutex_lock(&ch->mutex);

    while (ch->len == ch->cap) {
        pthread_cond_wait(&ch->not_full, &ch->mutex);
    }

    memcpy(
        ch->data + ch->tail * ch->elem_size,
        src,
        ch->elem_size
    );

    ch->tail = (ch->tail + 1) % ch->cap;
    ch->len++;

    pthread_cond_signal(&ch->not_empty);
    pthread_mutex_unlock(&ch->mutex);

    return 0;
}

void free_channel(channel *ch) {
    pthread_cond_destroy(&ch->not_full);
    pthread_cond_destroy(&ch->not_empty);
    pthread_mutex_destroy(&ch->mutex);
    free(ch);
}
