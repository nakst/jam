#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

int main(int argc, char **argv) {
	WIN32_FIND_DATA entry;
	HANDLE find = FindFirstFile("embed\\*.*", &entry);
	if (find == INVALID_HANDLE_VALUE) return 1;
	
	FILE *output = fopen("embed.h", "wb");
	FILE *def = fopen("embed_def.h", "wb");
	
	fprintf(def, "struct EmbedItem { const char *name; const void *pointer; size_t bytes; }; EmbedItem embedItems[1000]; void InitEmbed() {");
	
	int i = 0;
	do {
		if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
		
		char path[4096];
		sprintf(path, "embed\\%s", entry.cFileName);
		FILE *input = fopen(path, "rb");
		uint8_t buffer[4096];
		char file[4096];
		strcpy(file, strrchr(path, '\\') + 1);
		
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
	} while (FindNextFile(find, &entry));
	
	FindClose(find);
	fprintf(def, "}");
	return 0;
}
