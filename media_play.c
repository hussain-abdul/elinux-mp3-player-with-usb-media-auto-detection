#include <string.h>
#include "media_play.h"


pthread_cond_t cond;
pthread_mutex_t mut;


static char **media_files;
static int no_files = 0;

static gboolean bus_call(GstBus * bus, GstMessage * msg, gpointer data)
{
	GMainLoop *loop = (GMainLoop *) data;
	
	switch (GST_MESSAGE_TYPE(msg)) {
		
	case GST_MESSAGE_EOS:
		g_print("\nEnd of stream\n");
		g_main_loop_quit(loop);
		break;
		
	case GST_MESSAGE_ERROR:{
		gchar *debug;
		GError *error;
		
		gst_message_parse_error(msg, &error, &debug);
		g_free(debug);
		
		g_printerr("Error: %s\n", error->message);
		g_error_free(error);
		
		g_main_loop_quit(loop);
		break;
	}
	default:
		break;
	}
	
	return TRUE;
}


void play(char *song)
{
	GMainLoop *loop;
	GstElement *pipeline, *filesrc, *mad, *audioconvert, *alsasink;
	GstBus *bus;

	printf("song playing:%s\n", song);
	gst_init(NULL, NULL);
	loop = g_main_loop_new(NULL, FALSE);

	pipeline = gst_pipeline_new("mp3_player");
	
	filesrc = gst_element_factory_make("filesrc", "filesrc");
	mad = gst_element_factory_make("mad", "mad");
	audioconvert =
		gst_element_factory_make("audioconvert", "audioconvert");
	alsasink = gst_element_factory_make("alsasink", "alsasink");

	g_object_set(G_OBJECT(filesrc), "location", song, NULL);

	bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
	gst_bus_add_watch(bus, bus_call, loop);
	gst_object_unref(bus);

	gst_bin_add_many(GST_BIN(pipeline), filesrc, mad, audioconvert,
			 alsasink, NULL);
	gst_element_link_many(filesrc, mad, audioconvert, alsasink, NULL);

	gst_element_set_state(pipeline, GST_STATE_PLAYING);
	g_main_loop_run(loop);
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(pipeline);
}

void stop_play()
{
	no_files = 0;
}

void send_play_signal(int files, char **argv)
{
	no_files = files;
	media_files = argv;
	pthread_cond_signal(&cond);
	printf("play signal recieved\n");
}

void *start_play(void *ptr)
{
	int temp;
	int i;

	printf("waiting for signal\n");
	pthread_cond_wait(&cond, &mut);
	printf("playing started\n");
	while (1) {
		for (i = 0; i < no_files; i++) {
			play(media_files[i]);
			if (no_files == 0)
				break;
		}
		pthread_cond_wait(&cond, &mut);
	}
}
