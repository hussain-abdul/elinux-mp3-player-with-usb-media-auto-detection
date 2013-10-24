#include <stdio.h>
#include <pthread.h>
#include "media_search.h"
#include "media_mount.h"
#include "media_play.h"

int main()
{
	pthread_t listen_thread;
	pthread_t play_thread;

	printf("pid:%d\n", getpid());
	printf("device enumeration done\n");

	pthread_create(&play_thread, NULL, start_play, NULL);
	detect_device();
	pthread_create(&listen_thread, NULL, listen_device, NULL);


	pthread_join(listen_thread, NULL);
	pthread_join(play_thread, NULL);

	return 0;
}
