#include "platform_abstraction.h"
#include "mystructs.h"
#include "myfunc.h"
#include <stdio.h>
#include <string.h>

// ScreenCell ���� �ش��ϴ� ���ڿ��� ������ ��ȯ
static const char* cell_obj(ScreenCell cell) {
    switch (cell) {
    case C_wall:    return "��";
    case C_monster: return "\x1B[95m��\x1B[0m"; // Magenta
    case C_item:    return "\x1B[93m��\x1B[0m"; // Yellow
    case C_exit:    return "\x1B[96m��\x1B[0m"; // Cyan
    case C_user:  return "\x1B[91m��\x1B[0m"; // Red
    case C_null: return "  ";
    default:           return "  ";
    }
}

// back_buffer�� ���
void clear_back_buffer(Global* g) {
    int size = g->back_buffer->width * g->back_buffer->height;
    for (int i = 0; i < size; i++) {
        g->back_buffer->cells[i] = C_null;
    }
}

// ȭ���� �»�ܿ� �ش��ϴ� �̷� ��ǥ ���
static void maze_start(Global* g, int* x, int* y) {
    int cell_x = g->console_size.x / 2;
    int cell_y = g->console_size.y;
    // '���� ��' ������ ȭ�� �߾��� ���
    int center_cell_x = cell_x / 2;
    int center_cell_y = cell_y / 2;

    *x = g->player->pos.x - center_cell_x;
    *y = g->player->pos.y - center_cell_y;

}

// back_buffer�� front_buffer�� ���Ͽ� ����� �κи� ȭ�鿡 �׸�
void render_frame(Global* g) {
    ScreenBuffer* front = g->front_buffer;
    ScreenBuffer* back = g->back_buffer;

    for (int y = 0; y < back->height; y++) {
        for (int x = 0; x < back->width; x++) {
            int index = y * back->width + x;
            if (front->cells[index] != back->cells[index]) {
                gotoxy(x * 2, y); // wide character�� 2ĭ�� �����ϹǷ� x*2
                printf("%s", cell_obj(back->cells[index]));
                front->cells[index] = back->cells[index];
            }
        }
    }
    draw_ui(g); // UI�� �׻� �� ���� ���� �׸�
    hide_cursor();
}

void draw_maze(Global* g) {
    int cam_x, cam_y;
    maze_start(g, &cam_x, &cam_y);

    for (int y = 0; y < g->back_buffer->height; y++) {
        for (int x = 0; x < g->back_buffer->width; x++) {
            int maze_y = cam_y + y;
            int maze_x = cam_x + x;

            // �̷� ������ ����� �׸��� ���� (clear_back_buffer�� �̹� �����)
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
    // �÷��̾�� �׻� ȭ�� �߾ӿ� ��ġ
    int center_x = g->back_buffer->width / 2;
    int center_y = g->back_buffer->height / 2;
    g->back_buffer->cells[center_y * g->back_buffer->width + center_x] = C_user;
}

// UI�� ���۸� ������� �ʰ� ���� �׸�
void draw_ui(Global* g) {
    gotoxy(0, 0);
    // ȭ�� �� ������ �����ϰ� ����� �ٽ� ��
    printf("\x1B[K"); // ���� ���� Ŀ�� ������ ��� �����
    printf("Lv. %d | %s | HP: %d/100 | Monsters: %d | Items: %d",
        g->level, g->player->name, g->player->hp, g->monster_cnt, g->item_cnt);
}

void draw_game_drop(Global* g) {
    clear_screen();
    int center_x = g->console_size.x / 2;
    int center_y = g->console_size.y / 2;
    gotoxy(center_x - 10, center_y);
    printf("�����Ͻðڽ��ϱ�? (Y/N)");
    hide_cursor();
}