#include <metallc64.h>

long	calculate_offset(int track, int sector) {
	int sectors_before = (track - 1) * SECTORS_PER_TRACK;
	return (sectors_before + sector) * SECTOR_SIZE;
}

FILE	*dump_prg(FILE *d64file, int track, int sector, char *prg_file) {
	unsigned char buffer[SECTOR_SIZE];
	FILE *prg = fopen(prg_file, "w+b");
	if (!prg) {
		printf("failed to extract .prg file from .d64 disk image\n");
		return FALSE;
	}
	while (track != 0) {
		long offset = calculate_offset(track, sector);
		fseek(d64file, offset, SEEK_SET);
		memset(buffer, 0, sizeof(buffer));
		fread(buffer, 1, SECTOR_SIZE, d64file);

		unsigned char next_track = buffer[0];
		unsigned char next_sector = buffer[1];

		for (int i = 2; i < SECTOR_SIZE; i++)
			fwrite(&buffer[i], 1, 1, prg);

		track = next_track;
		sector = next_sector;
	}
	rewind(prg);
	return prg;
}

void	gen_name(char *name, char *pre_name, unsigned size) {
	unsigned pre_size = strlen(pre_name);
	if (pre_size + sizeof(name) > 0x400) return;
	memset(name, 0, 0x400);
	memcpy(name, pre_name, pre_size);
	for (unsigned i = 0; i < size; i++)
		name[pre_size + i] = (rand() % (90 - 65 + 1)) + 65;
}

FILE*	read_d64file(_bus *bus, FILE *d64file, char *file_path) {
	unsigned char dir_sector[SECTOR_SIZE];
	long dir_offset = calculate_offset(DIR_TRACK, DIR_SECTOR);

	if (fseek(d64file, dir_offset, SEEK_SET) < 0) {
		fclose(d64file);
		printf("invalid D64 disk image\n");
		return FALSE;
	}
	memset(dir_sector, 0, sizeof(dir_sector));
	unsigned chars_read = fread(dir_sector, 1, SECTOR_SIZE, d64file);
	if (!chars_read || chars_read != SECTOR_SIZE) {
		printf("invalid D64 disk image\n");
		fclose(d64file);
		return FALSE;
	}

	for (int i = 0; i < 8; i++) {
		unsigned entry_offset = i * 32;
		unsigned char file_type = dir_sector[entry_offset + 2];
		// type == 0x82 || 0x02 -> PRG file
		if ((file_type & 0x7) == 0x2) {
			int start_track = dir_sector[entry_offset + 3];
			int start_sector = dir_sector[entry_offset + 4];

			char prg_name[0x400];
			gen_name(prg_name, EXTRACTED_PRGS, 10);

			FILE *prg = dump_prg(d64file, start_track, start_sector, prg_name);
			fclose(d64file);
			if (!prg) return FALSE;

			memset(file_path, 0, 0x400);
			memcpy(file_path, prg_name, strlen(prg_name));
			return prg;
		}
	}
	printf("no PRG files found in \"%s\"\n", file_path);
	fclose(d64file);
	return FALSE;
}
