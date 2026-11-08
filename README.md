# channel

A bounded, blocking channel experiment in C using pthread mutexes and condition variables. It builds on ideas explored in [http-workers](https://github.com/choplib/http-workers), but stores **copies of elements** rather than a queue of pointers.

## Design

A fixed-capacity circular buffer stores elements of a configured `elem_size`. `channel_send` copies bytes from the producer into the buffer, while `channel_recv` copies them into a caller-provided destination. A mutex guards the head, tail, and length. Condition variables `not_empty` and `not_full` put receivers and senders to sleep while the channel is empty or full.

A channel owns its storage, not heap objects referenced by any pointer values copied into it.

## Declared public API

```c
channel *channel_create(size_t elem_size, size_t cap);
int channel_send(channel *ch, const void *src);
int channel_recv(channel *ch, void *dst);
void channel_destroy(channel *ch);
```

An intended usage pattern:

```c
channel *ch = channel_create(sizeof(int), 8);
int value = 42;
int received = 0;
channel_send(ch, &value);
channel_recv(ch, &received);
```

**Current API inconsistency:** `channel.h` declares `channel_destroy`, but `channel.c` implements `free_channel`. The public destruction function therefore does not link as declared. Fix that mismatch before integrating the module elsewhere.

## Limitations

- No channel close operation; blocked producers or consumers may wait indefinitely.
- Destroying a channel while it is in use, including by a waiting thread, is unsafe.
- No try-send, try-recv, or timeouts.
- No automated concurrency tests are included yet.
- Requires pthreads and is not intended as a complete replacement for Go channels.

Part of [choplib](https://github.com/choplib): experimental C building blocks.
