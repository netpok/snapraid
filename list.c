/*
 * Copyright (C) 2011 Andrea Mazzoleni
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "portable.h"

#include "util.h"
#include "elem.h"
#include "state.h"
#include "parity.h"
#include "handle.h"

/****************************************************************************/
/* list */

void state_list(struct snapraid_state* state)
{
	tommy_node* i;
	unsigned file_count;
	data_off_t file_size;
	unsigned link_count;

	file_count = 0;
	file_size = 0;
	link_count = 0;

	/* for each disk */
	for(i=state->disklist;i!=0;i=i->next) {
		tommy_node* j;
		struct snapraid_disk* disk = i->data;

		/* sort by name */
		tommy_list_sort(&disk->filelist, file_alpha_compare);

		/* for each file */
		for(j=disk->filelist;j!=0;j=j->next) {
			struct snapraid_file* file = j->data;
			struct tm tm_res;
			struct tm* tm;
			time_t t;

			++file_count;
			file_size += file->size;

			fprintf(stdlog, "file:%s:%s:%"PRIu64":%"PRIi64":%u:%"PRIi64"\n", disk->name, file->sub, file->size, file->mtime_sec, file->mtime_nsec, file->inode);

			t = file->mtime_sec;
			tm = localtime_r(&t, &tm_res);

			printf("%12"PRIu64" ", file->size);
			if (tm) {
				printf("%04u/%02u/%02u %02u:%02u ", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min);
			}
			printf("%s%s\n", disk->dir, file->sub);
		}

		/* for each link */
		for(j=disk->linklist;j!=0;j=j->next) {
			struct snapraid_link* link = j->data;
			const char* type;

			switch (link->flag &FILE_IS_LINK_MASK) {
			case FILE_IS_HARDLINK : type = "link"; break;
			case FILE_IS_SYMLINK : type = "symlink"; break;
			case FILE_IS_SYMDIR : type = "symdir"; break;
			case FILE_IS_JUNCTION : type = "junction"; break;
			default: type = "unknown"; break;
			}

			++link_count;

			fprintf(stdlog, "link_%s:%s:%s:%s\n", type, disk->name, link->sub, link->linkto);

			printf("%12s                  %s%s -> %s%s\n", type, disk->dir, link->sub, disk->dir, link->linkto);
		}
	}
	printf("\n");
	printf("%u files, for %"PRIu64" MiB.\n", file_count, file_size / (1024*1024));
	printf("%u links.\n", link_count);

	fprintf(stdlog, "summary:file_count:%u\n", file_count);
	fprintf(stdlog, "summary:file_size:%"PRIu64"\n", file_size);
	fprintf(stdlog, "summary:link_count:%u\n", link_count);
	fprintf(stdlog, "summary:exit:ok\n");
	fflush(stdlog);
}
