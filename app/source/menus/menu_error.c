#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "config.h"
#include "dialog.h"
#include "dirbrowse.h"
#include "glib2d_helper.h"
#include "status_bar.h"
#include "textures.h"
#include "utils.h"

void Menu_DisplayError(const char *msg, int ret) {
	char *result = (char *)calloc(64, sizeof(char));
	if (ret != 0)
		snprintf(result, 64, "Ret: %08X\n", ret);
	
	while (1) {
		Dialog_DisplayMessage("Error", msg, result);
		Utils_ReadControls();

		if ((Utils_IsButtonPressed(PSP_CTRL_ENTER)) || (Utils_IsButtonPressed(PSP_CTRL_CANCEL)))
			break;
	}

	free(result);
}
