#pragma once

#include <pspkernel.h>

typedef struct File {
	struct File *next; // Next item
	int isDir;         // Folder flag
	char name[256];    // File name
	char ext[5];       // File extension
	u64 size;          // File size
} File;

extern File *files;

extern int position;
extern int file_count;

extern u64 total_storage, used_storage;

int multi_select_index;           // Multi-select index.
bool multi_select[256];           // Array of indices selected.
int multi_select_indices[256];    // Array to hold the indices.
char multi_select_dir[512];       // Holds the current dir where multi-select happens.
char multi_select_paths[256][512]; // Holds the file paths of those in the clipboard.

void Dirbrowse_RecursiveFree(File *node);
int Dirbrowse_PopulateFiles(bool clear);
void Dirbrowse_DisplayFiles(void);
File *Dirbrowse_GetFileIndex(int index);
void Dirbrowse_OpenFile(void);
int Dirbrowse_Navigate(bool parent);
