#ifndef MEDIA_PLAY
#define MEDIA_PLAY

#include <gst/gst.h>

void play(char *song);
void *start_play(void *ptr);
void send_play_signal();
void stop_play();

#endif
