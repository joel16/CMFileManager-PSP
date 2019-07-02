#pragma once

void Dialog_DisplayMessage(const char *title, const char *msg_1, const char *msg_2);
void Dialog_DisplayPrompt(const char *title, const char *msg_1, const char *msg_2, int *selection, bool with_bg);
void Dialog_DisplayProgress(const char *title, const char *message, u32 offset, u32 size);
