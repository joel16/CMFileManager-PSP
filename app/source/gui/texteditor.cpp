#include <cstdlib>
#include <cstring>
#include <malloc.h>
#include <pspiofilemgr.h>
#include <pspthreadman.h>

#include "colours.h"
#include "config.h"
#include "fs.h"
#include "g2d.h"
#include "gui.h"
#include "log.h"
#include "textures.h"
#include "utils.h"

char font_size_cache[256] = {0};

/*
    This is a back-port of VITAShell's text editor by TheOfficialFloW.
    Original source can be found here: https://github.com/TheOfficialFloW/VitaShell/blob/master/text.c
    This only has very minor c++ code stlyle changes along with the PSP required changes.
*/

namespace TextViewer {
    // Text editor const vars
    constexpr int MAX_LINES = 0x10000;
    constexpr int MAX_LINE_CHARACTERS = 1024;
    constexpr int MAX_COPY_BUFFER_SIZE = 1024;
    constexpr int MAX_SELECTION = 1024;
    constexpr float TEXT_START_X = 55.f;
    constexpr int TAB_SIZE = 4;
    constexpr int SCREEN_WIDTH = 475;
    constexpr int SCREEN_HEIGHT =  272;
    constexpr float SHELL_MARGIN_X = 5.f;
    constexpr float SHELL_MARGIN_Y = 27.f;
    constexpr float MAX_WIDTH = (SCREEN_WIDTH - 5.f * SHELL_MARGIN_X);
    constexpr int MAX_POSITION = 17;
    constexpr int MAX_ENTRIES = 18;
    constexpr u64 BIG_BUFFER_SIZE = 16 * 1024 * 1024;
    constexpr float FONT_Y_SPACE = 12.f;
    constexpr float START_Y = (SHELL_MARGIN_Y + 3.f * FONT_Y_SPACE);

    // Scroll bar const vars
    constexpr float SCROLL_BAR_X = 476.f;
    constexpr float SCROLL_BAR_WIDTH = 4.f;
    constexpr float SCROLL_BAR_MIN_HEIGHT = 2.f;

    typedef struct TextListEntry {
        struct TextListEntry *next;
        struct TextListEntry *previous;
        int line_number;
        int selected;
        char line[MAX_LINE_CHARACTERS] = {0};
    } TextListEntry;
    
    typedef struct {
        TextListEntry *head;
        TextListEntry *tail;
        int length;
    } TextList;
    
    typedef struct CopyEntry {
        char line[MAX_LINE_CHARACTERS] = {0};
    } CopyEntry;
    
    typedef struct TextEditorState {
        int running;
        char *buffer;
        int size;
        int base_pos;
        int rel_pos;
        int offset_list[MAX_LINES] = {0};
        int selection_list[MAX_SELECTION] = {0};
        int n_selections;
        int n_copied_lines;
        int copy_reset;
        int modify_allowed;
        CopyEntry copy_buffer[MAX_COPY_BUFFER_SIZE] = {0};
        TextList list;
        int changed;
        int edit_line;
        int count_lines_thid;
        int hex_viewer;
        int count_lines_running;
        int n_lines;
    } TextEditorState;
    
    typedef struct CountParams {
        TextEditorState *state;
    } CountParams;

    enum TEXT_VIEWER_STATE {
        STATE_EDIT,
        STATE_DIALOG,
    };
    
    static void AddEntry(TextList *list, TextListEntry *entry) {
        entry->next = nullptr;
        entry->previous = nullptr;
        
        if (list->head == nullptr) {
            list->head = entry;
            list->tail = entry;
        }
        else {
            TextListEntry *tail = list->tail;
            tail->next = entry;
            entry->previous = tail;
            list->tail = entry;
        }
        
        list->length++;
    }
    
    static void EmptyList(TextList *list) {
        TextListEntry *entry = list->head;
        
        while (entry) {
            TextListEntry *next = entry->next;
            std::free(entry);
            entry = next;
        }
        
        list->head = nullptr;
        list->tail = nullptr;
        list->length = 0;
    }
    
    void DrawScrollbar(int pos, int n) {
        if (n > MAX_POSITION) {
            int START_Y_EXT = START_Y - 10.f;
            G2D::DrawRect(SCROLL_BAR_X, 52, SCROLL_BAR_WIDTH, 220, SELECTOR_COLOUR);
            
            float y = START_Y_EXT + ((pos * FONT_Y_SPACE) / (n * FONT_Y_SPACE)) * (MAX_ENTRIES * FONT_Y_SPACE);
            float height = ((MAX_POSITION * FONT_Y_SPACE) / (n * FONT_Y_SPACE)) * (MAX_ENTRIES * FONT_Y_SPACE);
            float scroll_bar_y = std::min(y, (START_Y_EXT + MAX_ENTRIES * FONT_Y_SPACE - height));
            G2D::DrawRect(SCROLL_BAR_X, scroll_bar_y, SCROLL_BAR_WIDTH, std::max(height, SCROLL_BAR_MIN_HEIGHT), TITLE_COLOUR);
        }
    }

    static int ReadLine(char *buffer, int offset, int size, char *line) {
        // Get line
        int line_width = 0;
        int count = 0;

        int i = 0;
        for (i = 0; i < std::min(size, std::min(size - offset, MAX_LINE_CHARACTERS - 1)); i++) {
            char ch = buffer[offset + i];
            char ch_width = 0;
            
            // Line break
            if (ch == '\n') {
                i++; // Skip it
                break;
            }
            
            // Tab
            if (ch == '\t')
                ch_width = TAB_SIZE * font_size_cache[' '];
            else {
                ch_width = font_size_cache[(int)ch];
                if (ch_width == 0) {
                    ch = ' '; // Change invalid characters to space
                    ch_width = font_size_cache[(int)ch];
                }
            }
            
            // Too long
            if ((line_width + ch_width) >= (MAX_WIDTH - TEXT_START_X + SHELL_MARGIN_X))
                break;
                
            // Increase line width
            line_width += ch_width;
            
            // Add to line string
            if (line)
                line[count++] = ch;
        }
        
        // End of line
        if (line)
            line[count] = '\0';
            
        return i;
    }
    
    static void UpdateEntry(TextEditorState *state, TextListEntry* entry, int rel_pos) {
        entry->line_number = state->base_pos + rel_pos;
        
        // Mark entry as selected
        entry->selected = 0;

        for (int j = 0; j < state->n_selections; j++) {
            if (entry->line_number == state->selection_list[j]) {
                entry->selected = 1;
                break;
            }
        }
        
        int length = TextViewer::ReadLine(state->buffer, state->offset_list[state->base_pos + rel_pos], state->size, entry->line);
        state->offset_list[state->base_pos + rel_pos + 1] = state->offset_list[state->base_pos + rel_pos] + length;
    }

    static void UpdateTextEntries(TextEditorState *state) {
        TextListEntry *entry = state->list.head;
        
        for (int i = 0; i < MAX_ENTRIES; i++) {
            if (!entry)
                break;
                
            TextViewer::UpdateEntry(state, entry, i);
            entry = entry->next;
        }
    }

    static void DeleteLine(TextEditorState *state, int line_number) {
        // Get current line
        int line_start = state->offset_list[line_number];
        char line[MAX_LINE_CHARACTERS] = {0};
        int length = TextViewer::ReadLine(state->buffer, line_start, state->size, line);
        
        // Remove line
        std::memmove(&state->buffer[line_start], &state->buffer[line_start + length], state->size - line_start);  
        state->size -= length;
        state->n_lines -= 1;
        
        // Add empty line if resulting buffer is empty
        if (state->size == 0) {
            state->size = 1;
            state->n_lines = 1;
            state->buffer[0] = '\n';
        }
        
        if (state->base_pos + state->rel_pos >= state->n_lines)
            state->rel_pos = state->n_lines - state->base_pos - 1;
            
        if (state->rel_pos < 0) {
            state->base_pos += state->rel_pos;
            state->rel_pos = 0;
        }
        
        state->changed = 1;
        state->n_selections = 0;
        
        // Update entries
        TextViewer::UpdateTextEntries(state);
    }

    static void InsertLine(TextEditorState *state, const char *line, int pos) {
        int offset = state->offset_list[pos];
        
        // calculated size of inserted line
        int length = std::strlen(line);
        
        // Make space for inserted line
        std::memmove(&state->buffer[offset + length], &state->buffer[offset], state->size - offset);
        state->size += length;
        
        // Insert the lines
        std::memcpy(&state->buffer[offset], line, length);
        
        for (int i = 0; i < length; i++) {
            if (line[i] == '\n')
                state->n_lines++;
        }
        
        state->n_selections = 0;
        state->changed = 1;
        state->copy_reset = 1;
        
        // Update entries
        TextViewer::UpdateTextEntries(state);
    }
    
    static int CountLinesThread(SceSize args, CountParams *params) {
        TextEditorState *state = params->state;
        state->count_lines_running = 1;
        state->n_lines = 0;
        
        int offset = 0;
        
        while (state->count_lines_running && offset < state->size && state->n_lines < MAX_LINES) {
            offset += TextViewer::ReadLine(state->buffer, offset, state->size, nullptr);
            state->n_lines++;
            sceKernelDelayThread(1000);
        }
        
        return sceKernelExitDeleteThread(0);
    }

    int Edit(const std::string &path) {
        TextEditorState *s = static_cast<TextEditorState *>(std::malloc(sizeof(TextEditorState)));
        if (!s)
            Log::Error("Text::Editor std::malloc: No memory!\n", path.c_str());

        char *buffer_base = static_cast<char *>(memalign(4096, BIG_BUFFER_SIZE));
        if (!buffer_base)
            Log::Error("Text::Editor std::memalign: No memory!\n", path.c_str());

        s->running = 1;
        s->hex_viewer = 0; 
        s->n_copied_lines = 0;
        s->copy_reset = 0;
        s->modify_allowed = 1;
        s->offset_list[0] = 0;
        s->count_lines_running = 0;
        s->n_lines = 0;
        s->edit_line = -1;

        s->size = FS::ReadFile(path, buffer_base, BIG_BUFFER_SIZE);
        
        if (s->size < 0) {
            std::free(buffer_base);
            return s->size;
        }
        
        s->buffer = buffer_base;
        
        int has_utf8_bom = 0;
        char utf8_bom[3] = {0xEF, 0xBB, 0xBF};
        if (s->size >= 3 && std::memcmp(buffer_base, utf8_bom, 3) == 0) {
            s->buffer += 3;
            has_utf8_bom = 1;
            s->size -= 3;
        }
        
        if (s->size == 0) {
            s->size = 1;
            s->buffer[0] = '\n';
        }
        
        if (s->buffer[s->size - 1] != '\n')
            s->buffer[s->size++] = '\n';
            
        s->base_pos = 0;
        s->rel_pos = 0;
        s->n_selections = 0;
        std::memset(&s->list, 0, sizeof(TextList));
        
        int i;
        for (i = 0; i < MAX_ENTRIES; i++) {
            TextListEntry *entry = static_cast<TextListEntry *>(std::malloc(sizeof(TextListEntry)));
            entry->line_number = i;
            entry->selected = 0;
            
            int length = TextViewer::ReadLine(s->buffer, s->offset_list[i], s->size, entry->line);
            s->offset_list[i + 1] = s->offset_list[i] + length;
            
            TextViewer::AddEntry(&s->list, entry);
        }
        
        CountParams count_params;
        count_params.state = s;
        
        s->count_lines_thid = sceKernelCreateThread("CountLinesThread", reinterpret_cast<SceKernelThreadEntry>(TextViewer::CountLinesThread), 0x12, 0x2000, PSP_THREAD_ATTR_USER, nullptr);
        if (R_SUCCEEDED(s->count_lines_thid))
            sceKernelStartThread(s->count_lines_thid, sizeof(CountParams), &count_params);
            
        s->edit_line = -1;
        s->changed = 0;

        std::string new_line = std::string();
        TEXT_VIEWER_STATE state = STATE_EDIT;
        std::string filename = FS::GetFilename(path);
        
        static int selection = 0;
        static const std::string prompt = "Do you wish to save your changes?";

        while (s->running) {
            int ctrl = Utils::ReadControls();

            if (state == STATE_DIALOG) {
                if (ctrl & PSP_CTRL_RIGHT)
                    selection++;
                else if (ctrl & PSP_CTRL_LEFT)
                    selection--;

                if (Utils::IsButtonPressed(PSP_CTRL_ENTER)) {
                    if (selection == 1) {
                        FS::WriteFile(path, buffer_base, has_utf8_bom ? s->size + sizeof(utf8_bom) : s->size);
                        break;
                    }
                    else
                        break;
                }
                
                Utils::SetBounds(selection, 0, 1);
            }
            else {
                if (ctrl & PSP_CTRL_UP) {
                    if (s->rel_pos > 0)
                        s->rel_pos--;
                    else {
                        if (s->base_pos > 0) {
                            s->base_pos--;
                            
                            // Tail to head
                            s->list.tail->next = s->list.head;
                            s->list.head->previous = s->list.tail;
                            s->list.head = s->list.tail;
                            
                            // Second last to tail
                            s->list.tail = s->list.tail->previous;
                            s->list.tail->next = nullptr;
                            
                            // No previous
                            s->list.head->previous = nullptr;
                            // Update line_number
                            s->list.head->line_number = s->base_pos;
                            
                            // Read
                            TextViewer::ReadLine(s->buffer, s->offset_list[s->base_pos], s->size, s->list.head->line);
                            
                            // Update the entry
                            TextViewer::UpdateEntry(s, s->list.head, 0);
                        }
                    }
                    
                    s->copy_reset = 1;
                }
                else if (ctrl & PSP_CTRL_DOWN) {
                    if (s->offset_list[s->rel_pos + 1] < s->size) {
                        if ((s->rel_pos + 1) < MAX_POSITION) {
                            if (s->base_pos + s->rel_pos < s->n_lines - 1) 
                                s->rel_pos++;
                        }
                        else {
                            if (s->offset_list[s->base_pos + s->rel_pos + 1] < s->size) {
                                s->base_pos++;
                                
                                // Head to tail
                                s->list.head->previous = s->list.tail;
                                s->list.tail->next = s->list.head;
                                s->list.tail = s->list.head;
                                
                                // Second first to head
                                s->list.head = s->list.head->next;
                                s->list.head->previous = nullptr;
                                
                                // No next
                                s->list.tail->next = nullptr;
                                
                                // Update line_number
                                s->list.tail->line_number = s->base_pos + MAX_ENTRIES - 1;
                                
                                // Read
                                int length = TextViewer::ReadLine(s->buffer, s->offset_list[s->base_pos + MAX_ENTRIES - 1], s->size, s->list.tail->line);
                                s->offset_list[s->base_pos + MAX_ENTRIES] = s->offset_list[s->base_pos + MAX_ENTRIES - 1] + length;
                                
                                // Update the entry
                                TextViewer::UpdateEntry(s, s->list.tail, MAX_ENTRIES - 1);
                            }
                        }
                    }
                    
                    s->copy_reset = 1;
                }
                else {
                    if ((ctrl & PSP_CTRL_LTRIGGER) || (ctrl & PSP_CTRL_RTRIGGER)) {
                        if (ctrl & PSP_CTRL_LTRIGGER) { // Skip page up
                            s->base_pos = s->base_pos - MAX_ENTRIES;
                            
                            if (s->base_pos < 0) {
                                s->base_pos = 0;
                                s->rel_pos = 0;
                            }
                        }
                        else { // Skip page down
                            s->base_pos = s->base_pos + MAX_ENTRIES;
                            if (s->base_pos >= s->n_lines - MAX_POSITION) {
                                s->base_pos = std::max(s->n_lines - MAX_POSITION, 0);
                                s->rel_pos = std::min(MAX_POSITION - 1, s->n_lines - 1);
                            }
                        }
                        
                        // Update entries
                        TextViewer::UpdateTextEntries(s);
                    }
                }
                
                // buffer modifying actions
                if (s->modify_allowed) {
                    if (s->edit_line <= 0 && Utils::IsButtonPressed(PSP_CTRL_ENTER)) {
                        int line_start = s->offset_list[s->base_pos + s->rel_pos];
                        
                        char line[MAX_LINE_CHARACTERS] = {0};
                        TextViewer::ReadLine(s->buffer, line_start, s->size, line);
                        
                        new_line = G2D::KeyboardGetText("Text editor", line);
                        s->edit_line = s->base_pos + s->rel_pos;
                    }
                    
                    // Delete line
                    if (Utils::IsButtonPressed(PSP_CTRL_LEFT) && s->n_copied_lines < MAX_COPY_BUFFER_SIZE)
                        TextViewer::DeleteLine(s, s->base_pos + s->rel_pos);
                        
                    // Insert new line
                    if (Utils::IsButtonPressed(PSP_CTRL_RIGHT))
                        TextViewer::InsertLine(s, "\n", s->base_pos + s->rel_pos + 1);
                        
                    if (Utils::IsButtonPressed(PSP_CTRL_CANCEL)) {
                        if (s->changed)
                            state = STATE_DIALOG;
                        else {
                            s->hex_viewer = 0;
                            break;
                        }
                    }
                }
            }

            if (s->edit_line >= 0) {
                if (!new_line.empty()) {
                    int line_start = s->offset_list[s->edit_line];
                    
                    char line[MAX_LINE_CHARACTERS] = {0};
                    int length = TextViewer::ReadLine(s->buffer, line_start, s->size, line);
                    
                    // Don't count newline
                    if (s->buffer[line_start + length - 1] == '\n')
                        length--;
                    
                    int new_length = new_line.size();
                    
                    // Move data if size has changed
                    if (new_length != length) {
                        std::memmove(&s->buffer[line_start + new_length], &s->buffer[line_start + length], s->size - line_start - length);
                        s->size += (new_length-length);
                    }
                    
                    // Copy new line into buffer
                    std::memcpy(&s->buffer[line_start], new_line.c_str(), new_length);
                    
                    // Add new lines to n_lines
                    for (int i = 0; i < new_length; i++) {
                        if (new_line.c_str()[i] == '\n')
                            s->n_lines++;
                    }
                    
                    // Update entries
                    TextViewer::UpdateTextEntries(s);
                    s->edit_line = -1;
                    s->changed = 1;
                }
                else
                    s->edit_line = -1;
            }

            // Start drawing
            g2dClear(BG_COLOUR);
            G2D::DrawRect(0, 0, 480, 18, STATUS_BAR_COLOUR);
            G2D::DrawRect(0, 18, 480, 34, MENU_BAR_COLOUR);
            GUI::DisplayStatusBar();
            G2D::DrawImage(icon_back, 5, 20);
            G2D::FontSetStyle(1.f, WHITE, INTRAFONT_ALIGN_LEFT);
            G2D::DrawText(40, 40, filename.c_str());
            
            TextViewer::DrawScrollbar(s->base_pos, s->n_lines);

            TextListEntry *entry = s->list.head;
            
            int i;
            for (i = 0; i < s->list.length; i++) {
                char *line = entry->line;
                
                if (entry->line_number < s->n_lines) {
                    char line_str[5] = {0};
                    std::snprintf(line_str, 5, "%04i", entry->line_number);
                    G2D::FontSetStyle(1.f, (s->rel_pos == i)? TITLE_COLOUR : TEXT_COLOUR, INTRAFONT_ALIGN_LEFT);
                    G2D::DrawText(SHELL_MARGIN_X, START_Y + (i * FONT_Y_SPACE), line_str);
                }
                
                float x = TEXT_START_X;
                
                if (entry->selected)
                    G2D::DrawRect(x, START_Y + (i * FONT_Y_SPACE) + 3.f, MAX_WIDTH - TEXT_START_X + SHELL_MARGIN_X, FONT_Y_SPACE, SELECTOR_COLOUR);

                G2D::DrawRect(47, 52, 1, 220, TITLE_COLOUR);
                    
                while (*line) {
                    char *p = std::strchr(line, '\t');
                    
                    if (p)
                        *p = '\0';
                    
                    G2D::FontSetStyle(1.f, (s->rel_pos == i)? TITLE_COLOUR : TEXT_COLOUR, INTRAFONT_ALIGN_LEFT);
                    int width = G2D::DrawText(x, START_Y + (i * FONT_Y_SPACE), line);
                    line += std::strlen(line);
                    
                    if (p) {
                        *p = '\t';
                        x += width + TAB_SIZE * font_size_cache[' '];
                        line++;
                    }
                }
                
                entry = entry->next;
            }

            if (state == STATE_DIALOG) {
                G2D::DrawRect(0, 18, 480, 254, G2D_RGBA(0, 0, 0, cfg.dark_theme? 50 : 80));
                G2D::DrawImage(dialog[cfg.dark_theme], ((480 - (dialog[0]->w)) / 2), ((272 - (dialog[0]->h)) / 2));
                G2D::FontSetStyle(1.f, TITLE_COLOUR, INTRAFONT_ALIGN_LEFT);
                G2D::DrawText(((480 - (dialog[0]->w)) / 2) + 10, ((272 - (dialog[0]->h)) / 2) + 20, "Save");
                
                int confirm_width = intraFontMeasureText(font, "YES");
                int cancel_width = intraFontMeasureText(font, "NO");
                
                if (selection == 0)
                    G2D::DrawRect((364 - cancel_width) - 5, (180 - (font->texYSize - 15)) - 5, cancel_width + 10, (font->texYSize - 5) + 10, SELECTOR_COLOUR);
                else
                    G2D::DrawRect((409 - (confirm_width)) - 5, (180 - (font->texYSize - 15)) - 5, confirm_width + 10, (font->texYSize - 5) + 10, SELECTOR_COLOUR);
                    
                G2D::DrawText(409 - (confirm_width), (192 - (font->texYSize - 15)) - 3, "YES");
                G2D::DrawText(364 - cancel_width, (192 - (font->texYSize - 15)) - 3, "NO");
                
                int prompt_width = intraFontMeasureText(font, prompt.c_str());
                G2D::FontSetStyle(1.f, TEXT_COLOUR, INTRAFONT_ALIGN_LEFT);
                G2D::DrawText(((480 - (prompt_width)) / 2), ((272 - (dialog[0]->h)) / 2) + 60, prompt.c_str());
            }

            g2dFlip(G2D_VSYNC);
        }
        
        s->count_lines_running = 0;
        sceKernelWaitThreadEnd(s->count_lines_thid, nullptr);
        TextViewer::EmptyList(&s->list);
        std::free(s);
        std::free(buffer_base); 
        return 0;
    }
}
