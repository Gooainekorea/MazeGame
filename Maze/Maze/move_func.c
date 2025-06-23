// #include "platform_abstraction.h"
#include "mystructs.h"
#include "myfunc.h"
// #include <stdio.h>
#include <stdlib.h>
// #include <time.h>
// #include <string.h>
// #include <stdarg.h>

// �̷� �̵� �� �÷��̾� �浹 ó�� �Լ�
void move_maze(Global* g, int direction) {
    int next_x = g->player->pos.x;
    int next_y = g->player->pos.y;

    switch (direction) {
    case 0: next_y--; break; // ��
    case 1: next_y++; break; // ��
    case 2: next_x--; break; // ��
    case 3: next_x++; break; // ��
    }

    if (next_x >= 0 && next_x < g->maze_size && next_y >= 0 && next_y < g->maze_size) {
        if (g->maze[next_y][next_x].cell == 0) {
            g->player->pos.x = next_x;
            g->player->pos.y = next_y;
        }
    }
}
// �̷ο��� ���ƴٴϴ°͵� ����ó��-�ڵ����� ���ƴٴϰ�
static void turn_element(Global* g, Move_element* ele) {//������ȯ
    int dx[] = { 0, 0, -1, 1 };
    int dy[] = { -1, 1, 0, 0 };

    int possible_moves[4];
    int count = 0;

    for (int i = 0; i < 4; i++) {
        int next_x = ele->pos.x + dx[i];
        int next_y = ele->pos.y + dy[i];
        if (next_x >= 0 && next_x < g->maze_size && next_y >= 0 && next_y < g->maze_size && g->maze[next_y][next_x].cell == 0) {
            possible_moves[count++] = i;
        }
    }

    if (count > 0) {
        ele->direction = possible_moves[rand() % count];
        ele->distance = rand() % (g->maze_size / 4) + 3;
    }
}
void move_element(Global* g, Move_element* ele) {//����
    if (ele->distance <= 0) {
        turn_element(g, ele);
        return;
    }

    int next_x = ele->pos.x;
    int next_y = ele->pos.y;

    switch (ele->direction) {
    case 0: next_y--; break;
    case 1: next_y++; break;
    case 2: next_x--; break;
    case 3: next_x++; break;
    }

    if (next_x >= 0 && next_x < g->maze_size && next_y >= 0 && next_y < g->maze_size && g->maze[next_y][next_x].cell == 0) {
        ele->pos.x = next_x;
        ele->pos.y = next_y;
        ele->distance--;
    }
    else {
        turn_element(g, ele);
    }
}