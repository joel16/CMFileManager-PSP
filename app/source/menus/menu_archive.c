#include <archive.h>
#include <archive_entry.h>
#include <malloc.h>

#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "glib2d_helper.h"
#include "fs.h"
#include "menu_error.h"
#include "progress_bar.h"
#include "status_bar.h"
#include "textures.h"
#include "utils.h"

static char *Archive_RemoveFileExt(char *filename) {
	char *ret, *lastdot;

   	if (filename == NULL)
   		return NULL;
   	if ((ret = malloc(strlen(filename) + 1)) == NULL)
   		return NULL;

   	strcpy(ret, filename);
   	lastdot = strrchr(ret, '.');

   	if (lastdot != NULL)
   		*lastdot = '\0';

   	return ret;
}

static u64 Archive_CountFiles(const char *path) {
	int ret = 0;
	u64 count = 0;
	
	struct archive *handle = archive_read_new();
	ret = archive_read_support_format_all(handle);
	ret = archive_read_open_filename(handle, path, 1024);
	
	if (ret != ARCHIVE_OK) {
		Menu_DisplayError("archive_read_open_filename :", ret);
		return 1;
	}
	
	for (;;) {
		struct archive_entry *entry = NULL;
		int ret = archive_read_next_header(handle, &entry);
		if (ret == ARCHIVE_EOF)
			break;
		
		count++;
	}
	
	ret = archive_read_free(handle);
	return count;
}

static int Archive_WriteData(struct archive *src, struct archive *dst) {
	int ret = 0;
	
	for (;;) {
		const void *chunk = NULL;
		size_t length = 0;
		s64 offset = 0;
		
		ret = archive_read_data_block(src, &chunk, &length, &offset);
		if (ret == ARCHIVE_EOF)
			return ARCHIVE_OK;
		
		if (ret != ARCHIVE_OK)
			return ret;
			
		ret = archive_write_data_block(dst, chunk, length, offset);
		if (ret != ARCHIVE_OK)
			return ret;
	}
	
	return -1;
}

int Archive_ExtractArchive(const char *path) {
	char *dest_path = malloc(256);
	char *dirname_without_ext = Archive_RemoveFileExt((char *)path);
	
	snprintf(dest_path, 512, "%s/", dirname_without_ext);
	FS_MakeDir(dest_path);
	
	int count = 0;
	u64 max = Archive_CountFiles(path);
	
	int ret = 0;
	struct archive *handle = archive_read_new();
	struct archive *dst = archive_write_disk_new();
	
	ret = archive_read_support_format_all(handle);
	ret = archive_read_open_filename(handle, path, 1024);
	if (ret != ARCHIVE_OK) {
		Menu_DisplayError("archive_read_open_filename :", ret);
		return 1;
	}
	
	for (;;) {
		ProgressBar_DisplayProgress("Extracting", path, count, max);
		
		struct archive_entry *entry = NULL;
		ret = archive_read_next_header(handle, &entry);
		if (ret == ARCHIVE_EOF)
			break;
			
		if (ret != ARCHIVE_OK) {
			Menu_DisplayError("archive_read_next_header failed:", ret);
			
			if (ret != ARCHIVE_WARN)
				break;
		}
		
		const char *entry_path = archive_entry_pathname(entry);
		char new_path[1024];

		ret = snprintf(new_path, sizeof(new_path), "%s/%s", dest_path, entry_path);
		ret = archive_entry_update_pathname_utf8(entry, new_path);
		if (!ret) {
			Menu_DisplayError("archive_entry_update_pathname_utf8:", ret);
			break;
		}
		
		ret = archive_write_disk_set_options(dst, ARCHIVE_EXTRACT_UNLINK);
		ret = archive_write_header(dst, entry);
		if (ret != ARCHIVE_OK) {
			Menu_DisplayError("archive_write_header failed:", ret);
			break;
		}
		
		ret = Archive_WriteData(handle, dst);
		ret = archive_write_finish_entry(dst);
		count++;
	}
	
	ret = archive_write_free(dst);
	ret = archive_read_free(handle);
	free(dest_path);
	free(dirname_without_ext);
	return ret;
}

int Archive_ExtractFile(const char *path) {
	int dialog_selection = 0;
	int text_width1 = intraFontMeasureText(font, "This may take a few minutes.");
	int text_width2 = intraFontMeasureText(font, "Do you want to continue?");

	while (1) {
		g2dClear(config.dark_theme? BLACK_BG : WHITE);
		G2D_DrawRect(0, 0, 480, 20, config.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT);
		G2D_DrawRect(0, 20, 480, 42, config.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);
		G2D_DrawImage(icon_nav_drawer, 5, 25);

		StatusBar_DisplayTime();
		Dirbrowse_DisplayFiles();

		G2D_DrawRect(0, 20, 480, 252, G2D_RGBA(0, 0, 0, config.dark_theme? 50: 80));

		G2D_DrawImage(config.dark_theme? dialog_dark : dialog, ((480 - dialog->w) / 2), ((272 - dialog->h) / 2));

		intraFontSetStyle(font, 0.7f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
		intraFontPrint(font, ((480 - dialog->w) / 2) + 10, ((272 - dialog->h) / 2) + 20, "Extract file");

		intraFontSetStyle(font, 0.7f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
		intraFontPrint(font, ((480 - (text_width1)) / 2), ((272 - dialog->h) / 2) + 50, "This may take a few minutes.");
		intraFontPrint(font, ((480 - (text_width2)) / 2), ((272 - dialog->h) / 2) + 66, "Do you wish to continue?");

		intraFontSetStyle(font, 0.7f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);

		if (dialog_selection == 0)
			G2D_DrawRect((364 - intraFontMeasureText(font, "NO")) - 5, (180 - (font->texYSize - 20)) - 5, intraFontMeasureText(font, "NO") + 10, (font->texYSize - 10) + 10, 
				config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
		else if (dialog_selection == 1)
			G2D_DrawRect((409 - (intraFontMeasureText(font, "YES"))) - 5, (180 - (font->texYSize - 20)) - 5, intraFontMeasureText(font, "YES") + 10, (font->texYSize - 10) + 10, 
				config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

		intraFontPrint(font, 409 - (intraFontMeasureText(font, "YES")), (182 - (font->texYSize - 30)), "YES");
		intraFontPrint(font, 364 - intraFontMeasureText(font, "NO"), (182 - (font->texYSize - 30)), "NO");

		g2dFlip(G2D_VSYNC);

		Utils_ReadControls();

		if (Utils_IsButtonPressed(PSP_CTRL_RIGHT))
			dialog_selection++;
		else if (Utils_IsButtonPressed(PSP_CTRL_LEFT))
			dialog_selection--;

		Utils_SetMax(&dialog_selection, 0, 1);
		Utils_SetMin(&dialog_selection, 1, 0);

		if (Utils_IsButtonPressed(PSP_CTRL_CANCEL))
			break;

		if (Utils_IsButtonPressed(PSP_CTRL_ENTER)) {
			if (dialog_selection == 1) {
				return Archive_ExtractArchive(path);
			}
			else
				break;
		}
	}

	return -1;
}
