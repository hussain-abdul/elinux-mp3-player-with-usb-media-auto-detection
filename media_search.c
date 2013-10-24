#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include "media_search.h"

int check_mp3(char *name)
{
	char buf[4];
	int i;

	while (*name != '\0')
		name++;
	name = name - 3;
	for (i = 0; i < 3; i++) {
		buf[i] = *name;
		name++;
	}
	buf[i] = '\0';
	return strcmp(buf, "mp3");
}

int search(char *song_path, char **media_files)
{
	DIR *dir, *subdir;
	struct dirent *entry, *sub_entry;
	char buf[PATH_MAX];
	//char *media_files[256];
	int file_counter = 0;

	printf("song path:%s\n", song_path);
	dir = opendir(song_path);
	if (dir == NULL) {
		error(1, error, "error in opening directory");
	}

	while (entry = readdir(dir)) {

		if (entry->d_type == 4 && strncmp(entry->d_name, ".", 1)
		    && strncmp(entry->d_name, "..", 2)) {
			snprintf(buf, PATH_MAX, "%s%s", song_path, entry->d_name);
			//                      printf("%s\n",buf);
			subdir = opendir(buf);
			if (subdir == NULL) {
				error(1, error, "error in opening directory");
			}
			while (sub_entry = readdir(subdir)) {
				if (!check_mp3(sub_entry->d_name)) {
					//      printf("    %s\n",sub_entry->d_name);
					media_files[file_counter] = (char *) malloc(PATH_MAX);
					sprintf(media_files[file_counter], "%s/%s", buf,
						sub_entry->d_name);
					file_counter++;

				}

			}
		} else {

			if (!check_mp3(entry->d_name)) {
				//              printf("%s\n",entry->d_name);
				media_files[file_counter] = (char *) malloc(PATH_MAX);
				snprintf(buf, PATH_MAX, "%s%s", song_path, entry->d_name);
				strcpy(media_files[file_counter], buf);
				file_counter++;
			}

		}
	}
	//      printf("files present = %d\n",file_counter);

	return file_counter;
}
