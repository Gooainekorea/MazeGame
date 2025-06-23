#include "platform_abstraction.h"
#include "mystructs.h"
#include "myfunc.h"
#include <stdio.h>
#include <string.h>

// ScreenCell 값에 해당하는 문자열과 색상을 반환
static const char* cell_obj(ScreenCell cell) {
    switch (cell) {
    case C_wall:    return "■";
    case C_monster: return "\x1B[95m※\x1B[0m"; // Magenta
    case C_item:    return "\x1B[93m♬\x1B[0m"; // Yellow
    case C_exit:    return "\x1B[96m★\x1B[0m"; // Cyan
    case C_user:  return "\x1B[91m●\x1B[0m"; // Red
    case C_null: return "  ";
    default:           return "  ";
    }
}

// back_buffer를 비움
void clear_back_buffer(Global* g) {
    int size = g->back_buffer->width * g->back_buffer->height;
    for (int i = 0; i < size; i++) {
        g->back_buffer->cells[i] = C_null;
    }
}

// 화면의 좌상단에 해당하는 미로 좌표 계산
static void maze_start(Global* g, int* x, int* y) {
    int cell_x = g->console_size.x / 2;
    int cell_y = g->console_size.y;
    // '게임 셀' 단위의 화면 중앙을 계산
    int center_cell_x = cell_x / 2;
    int center_cell_y = cell_y / 2;

    *x = g->player->pos.x - center_cell_x;
    *y = g->player->pos.y - center_cell_y;

}

// back_buffer와 front_buffer를 비교하여 변경된 부분만 화면에 그림
void render_frame(Global* g) {
    ScreenBuffer* front = g->front_buffer;
    ScreenBuffer* back = g->back_buffer;

    for (int y = 0; y < back->height; y++) {
        for (int x = 0; x < back->width; x++) {
            int index = y * back->width + x;
            if (front->cells[index] != back->cells[index]) {
                gotoxy(x * 2, y); // wide character는 2칸을 차지하므로 x*2
                printf("%s", cell_obj(back->cells[index]));
                front->cells[index] = back->cells[index];
            }
        }
    }
    draw_ui(g); // UI는 항상 맨 위에 새로 그림
    hide_cursor();
}

void draw_maze(Global* g) {
    int cam_x, cam_y;
    maze_start(g, &cam_x, &cam_y);

    for (int y = 0; y < g->back_buffer->height; y++) {
        for (int x = 0; x < g->back_buffer->width; x++) {
            int maze_y = cam_y + y;
            int maze_x = cam_x + x;

            // 미로 범위를 벗어나면 그리지 않음 (clear_back_buffer가 이미 비워둠)
            if (maze_y >= 0 && maze_y < g->maze_size && maze_x >= 0 && maze_x < g->maze_size) {
                if (g->maze[maze_y][maze_x].cell) {
                    g->back_buffer->cells[y * g->back_buffer->width + x] = C_wall;
                }
            }
        }
    }
}

void draw_monsters(Global* g) {
    int cam_x, cam_y;
    maze_start(g, &cam_x, &cam_y);

    for (int i = 0; i < g->monster_cnt; i++) {
        Position* pos = &g->monsters[i].move.pos;
        int screen_x = pos->x - cam_x;
        int screen_y = pos->y - cam_y;

        if (screen_y >= 0 && screen_y < g->back_buffer->height &&
            screen_x >= 0 && screen_x < g->back_buffer->width) {
            g->back_buffer->cells[screen_y * g->back_buffer->width + screen_x] = C_monster;
        }
    }
}

void draw_items(Global* g) {
    int cam_x, cam_y;
    maze_start(g, &cam_x, &cam_y);

    Item* node = g->item_list_h;
    while (node != NULL) {
        int screen_x = node->pos.x - cam_x;
        int screen_y = node->pos.y - cam_y;

        if (screen_y >= 0 && screen_y < g->back_buffer->height &&
            screen_x >= 0 && screen_x < g->back_buffer->width) {
            g->back_buffer->cells[screen_y * g->back_buffer->width + screen_x] = C_item;
        }
        node = node->next;
    }
}

void draw_exit(Global* g) {
    int cam_x, cam_y;
    maze_start(g, &cam_x, &cam_y);

    Position* pos = &g->exit->pos;
    int screen_x = pos->x - cam_x;
    int screen_y = pos->y - cam_y;

    if (screen_y >= 0 && screen_y < g->back_buffer->height &&
        screen_x >= 0 && screen_x < g->back_buffer->width) {
        g->back_buffer->cells[screen_y * g->back_buffer->width + screen_x] = C_exit;
    }
}

void draw_user(Global* g) {
    // 플레이어는 항상 화면 중앙에 위치
    int center_x = g->back_buffer->width / 2;
    int center_y = g->back_buffer->height / 2;
    g->back_buffer->cells[center_y * g->back_buffer->width + center_x] = C_user;
}

// UI는 버퍼를 사용하지 않고 직접 그림
void draw_ui(Global* g) {
    gotoxy(0, 0);
    // 화면 맨 윗줄을 깨끗하게 지우고 다시 씀
    printf("\x1B[K"); // 현재 줄의 커서 오른쪽 모두 지우기
    printf("Lv. %d | %s | HP: %d/100 | Monsters: %d | Items: %d",
        g->level, g->player->name, g->player->hp, g->monster_cnt, g->item_cnt);
}

void draw_game_drop(Global* g) {
    clear_screen();
    int center_x = g->console_size.x / 2;
    int center_y = g->console_size.y / 2;
    gotoxy(center_x - 10, center_y);
    printf("종료하시겠습니까? (Y/N)");
    hide_cursor();
}