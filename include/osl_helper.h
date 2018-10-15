#pragma once

void OSL_StartDrawing(void);
void OSL_EndDrawing(void);
void OSL_DrawFillRect(int x, int y, int w, int h, OSL_COLOR color);
void OSL_DisplayKeyboard(char *descStr, char *initialStr, char *text);
