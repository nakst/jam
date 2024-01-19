#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

int main(int argc, char **argv) {
#ifdef _WIN32
	WIN32_FIND_DATA entry;
	HANDLE find = FindFirstFile("embed\\*.*", &entry);
	if (find == INVALID_HANDLE_VALUE) return 1;
#else
	struct dirent *entry;
	DIR *find = opendir("embed/");
#endif
	
	FILE *output = fopen("bin/embed.h", "wb");
	FILE *def = fopen("bin/embed_def.h", "wb");
	
	fprintf(def, "typedef struct EmbedItem { const char *name; const void *pointer; size_t bytes; } EmbedItem; EmbedItem embedItems[1000]; void InitEmbed() {");
	
	int i = 0;
	do {
		char path[4096];
#ifdef _WIN32
		if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
		sprintf(path, "embed\\%s", entry.cFileName);
#else
		entry = readdir(find);
		if (!entry) break;
		sprintf(path, "embed/%s", entry->d_name);
		struct stat s;
		stat(path, &s);
		if (!S_ISREG(s.st_mode)) continue;
#endif

		FILE *input = fopen(path, "rb");
		uint8_t buffer[4096];
		char file[4096];
#ifdef _WIN32
		strcpy(file, strrchr(path, '\\') + 1);
#else
		strcpy(file, strrchr(path, '/') + 1);
#endif
		
		for (int i = 0; file[i]; i++) {
			if (!isalpha(file[i]) && !isdigit(file[i])) {
				file[i] = '_';
			}
		}
		
		fprintf(output, "const uint8_t %s[] = { ", file);
		int total = 0;
		
		while (true) {
			int bytes = fread(buffer, 1, sizeof(buffer), input);
			if (!bytes) break;
			total += bytes;
			
			for (int i = 0; i < bytes; i++) {
				fprintf(output, "0x%.2X, ", buffer[i]);
			}
		}
		
		fprintf(output, "};\nsize_t %sBytes = %d;\n", file, total);
		// fprintf(def, "extern uint8_t %s[]; extern size_t %sBytes;", file, file);
		fprintf(def, "embedItems[%d].name = \"%s\"; embedItems[%d].pointer = %s; embedItems[%d].bytes = %sBytes;", i, file, i, file, i, file);
		fclose(input);
		i++;
	}
#ifdef _WIN32
	while (FindNextFile(find, &entry));
#else
	while (true);
#endif
	
#ifdef _WIN32
	FindClose(find);
#else
	closedir(find);
#endif
	fprintf(def, "}");
	return 0;
}
