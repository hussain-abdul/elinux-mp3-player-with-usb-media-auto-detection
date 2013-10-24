#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mount.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <libudev.h>
#include <unistd.h>
#include <dirent.h>
#include <semaphore.h>

#include "media_mount.h"
#include "media_search.h"
#include "media_play.h"

const char *path;
static struct udev *udev;
static struct udev_device *dev, *pdev;

char *media_files[256];
int no_files;
static int search_flag = 0;

void print_mp3_files(int total_files)
{
	int i;

	for (i = 0; i < total_files; i++)
		printf("%s\n", media_files[i]);
}

void free_media_data(int total_files)
{
	int i;

	for (i = 0; i < total_files; i++)
		free(media_files[i]);
}

void detect_device()
{
	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;
	int ret;
	pid_t pid;
	int status;

	path = (char *) malloc(PATH_MAX);
	udev = udev_new();
	if (!udev) {
		printf("can't create udev\n");
		exit(1);
	}

	enumerate = udev_enumerate_new(udev);
	udev_enumerate_add_match_subsystem(enumerate, "block");
	udev_enumerate_scan_devices(enumerate);
	devices = udev_enumerate_get_list_entry(enumerate);

	udev_list_entry_foreach(dev_list_entry, devices) {

		path = udev_list_entry_get_name(dev_list_entry);
		dev = udev_device_new_from_syspath(udev, path);

		pdev =
			udev_device_get_parent_with_subsystem_devtype(dev, "usb",
								      "usb_device");
		if (pdev) {
			path = udev_device_get_devnode(dev);
			printf("Device node path : %s\n", path);
			pid = fork();
			if (pid == 0) {
				execlp("umount", "umount", "/mnt", NULL);
			}
			wait(&status);
			pid = fork();
			if (pid == 0) {
				execlp("mount", "mount", path, "/mnt", NULL);
			}
			wait(&status);
			no_files = search("/mnt/", media_files);
			printf("number of files = %d\n", no_files);
			print_mp3_files(no_files);
			if (no_files != 0)
				send_play_signal(no_files, media_files);
		}
		udev_device_unref(dev);
	}
	udev_enumerate_unref(enumerate);
	udev_unref(udev);
}

void *listen_device(void *ptr)
{
	int fd;
	fd_set fds;
	int ret;
	struct udev_monitor *mon;
	const char *action;
	pid_t pid;
	int status;

	action = (char *) malloc(512);
	udev = udev_new();
	if (!udev) {
		printf("can't create udev\n");
		exit(1);
	}
	mon = udev_monitor_new_from_netlink(udev, "udev");
	udev_monitor_filter_add_match_subsystem_devtype(mon, "block", NULL);
	udev_monitor_enable_receiving(mon);

	fd = udev_monitor_get_fd(mon);
	printf("fd = %d\n", fd);
	while (1) {
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		printf("waiting for device action...\n");
		ret = select(fd + 1, &fds, NULL, NULL, NULL);
		if (ret > 0 && FD_ISSET(fd, &fds)) {
			dev = udev_monitor_receive_device(mon);
			path = udev_device_get_devnode(dev);
			action = udev_device_get_action(dev);
			printf("device %s : %s\n", action, path);
			if (strncmp(action, "remove", 6)) {
				printf("add device\n");
				pid = fork();
				if (pid == 0) {
					execlp("umount", "umount", "/mnt", NULL);
				}
				wait(&status);
				pid = fork();
				if (pid == 0) {
					execlp("mount", "mount", path, "/mnt", NULL);
				}
				wait(&status);
				no_files = search("/mnt/", media_files);
				printf("no of files = %d\n", no_files);
				print_mp3_files(no_files);
				if (no_files != 0)
					send_play_signal(no_files, media_files);
			} else {
				printf("remove device\n");
				if ((no_files != 0)) {
					free_media_data(no_files);
				}
				stop_play();
			}
		} else {
			printf("return value = %d\n", ret);
			return NULL;
		}
	}
}
