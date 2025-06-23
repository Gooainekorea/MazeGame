#include "platform_abstraction.h"
#include "mystructs.h"
#include "myfunc.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

// Ű ���� ������ (���� ���ʿ�, ���� �ڵ� ��ȣ)
void* key_deliver_thread_func(void* argument) {
    Global* g = (Global*)argument;
    int key_pressed = 0;

    while (1) {
        lock_read_lock(&g->lock);
        Game_state current_state = g->game_state;
        lock_read_unlock(&g->lock);

        if (current_state == GameOver) {
            break;
        }

        if (mykbhit()) {
            key_pressed = mygetch();
#ifdef _WIN32
            if (key_pressed == 224 || key_pressed == 0) {
                key_pressed = mygetch();
            }
#endif
            lock_write_lock(&g->lock);
            g->key = key_pressed;
            lock_write_unlock(&g->lock);
        }
        mysleep(10);
    }
    return NULL;
}

// ===[������ ���� ���� ������]===
void* game_logic_thread_func(void* argument) {
    Global* g = (Global*)argument;
    while (1) {
        int should_break = 0;

        // ---[ũ��Ƽ�� ���� ����]---
        // ���� ������(g)�� �а� ���� ��� ������ �ϳ��� ��� ��� �ȿ��� ó���մϴ�.
        // �̸� ���� �������� �ϰ����� �����մϴ�.
        lock_write_lock(&g->lock);

        // ���� ���¿� ó���� Ű�� ���� ������ �����ɴϴ�.
        Game_state current_state = g->game_state;
        int key_to_process = g->key;

        // Ű�� ó�������Ƿ� ���� ������ 0���� �����մϴ�.
        if (key_to_process != 0) {
            g->key = 0;
        }

        // 1. ���¿� ���� Ű �Է� ó��
        if (current_state == GameRun) {
            if (key_to_process == 27) { // ESC Ű
                g->game_state = GameStop;
            }

            // ����Ű ó�� (�÷��̾� �̵�)
            switch (key_to_process) {
            case 72: move_maze(g, 0); break; // UP
            case 80: move_maze(g, 1); break; // DOWN
            case 75: move_maze(g, 2); break; // LEFT
            case 77: move_maze(g, 3); break; // RIGHT
#ifndef _WIN32
                /*#define KEY_UP    259
                #define KEY_DOWN  258
                #define KEY_LEFT  260
                #define KEY_RIGHT 261*/

            case KEY_UP:    move_maze(g, 0); break;
            case KEY_DOWN:  move_maze(g, 1); break;
            case KEY_LEFT:  move_maze(g, 2); break;
            case KEY_RIGHT: move_maze(g, 3); break;
#endif
            }
        }
        else if (current_state == GameStop) {
            if (key_to_process == 'y' || key_to_process == 'Y') {
                g->game_state = GameOver;
            }
            else if (key_to_process == 'n' || key_to_process == 'N') {
                g->game_state = GameRun;
            }
        }

        // 2. ���� ���� ������Ʈ (GameRun ������ ����)
        if (g->game_state == GameRun) {
            long long current_time = get_current_time_ms();

            // ���� �̵�
            for (int i = 0; i < g->monster_cnt; i++) {
                if (g->monsters[i].move.speed <= 0) continue;
                long move_interval = 1000 / g->monsters[i].move.speed;
                if (current_time - g->monsters[i].move.last_move_time > move_interval) {
                    Position a = g->monsters[i].move.pos;
                    move_element(g, &g->monsters[i].move);
                    Position b = g->monsters[i].move.pos;
                    g->maze[a.y][a.x].monster = 0;
                    g->maze[b.y][b.x].monster = 1;
                    g->monsters[i].move.last_move_time = current_time;
                }
            }

            // �ⱸ �̵�
            if (g->exit->speed > 0) {
                long exit_move_interval = 1000 / g->exit->speed;
                if (current_time - g->exit->last_move_time > exit_move_interval) {
                    Position a = g->exit->pos;
                    move_element(g, g->exit);
                    Position b = g->exit->pos;
                    g->maze[a.y][a.x].exit = 0;
                    g->maze[b.y][b.x].exit = 1;
                    g->exit->last_move_time = current_time;
                }
            }

            // �浹 ó��: ����
            for (int i = 0; i < g->monster_cnt; i++) {
                if (g->player->pos.x == g->monsters[i].move.pos.x && g->player->pos.y == g->monsters[i].move.pos.y) {
                    g->player->hp -= g->monsters[i].attack;
                    if (g->player->hp <= 0) {
                        g->game_state = GameOver;
                    }
                }
            }

            // �浹 ó��: ������
            Item* current_item = g->item_list_h;
            Item* prev_item = NULL;
            while (current_item != NULL) {
                if (current_item->pos.x == g->player->pos.x && current_item->pos.y == g->player->pos.y) {
                    g->player->hp += current_item->hp_recvr;
                    Position a = current_item->pos;
                    g->maze[a.y][a.x].item = 0;
                    g->item_cnt--;

                    if (prev_item == NULL) {
                        g->item_list_h = current_item->next;
                        free(current_item);
                        current_item = g->item_list_h;
                    }
                    else {
                        prev_item->next = current_item->next;
                        free(current_item);
                        current_item = prev_item->next;
                    }
                }
                else {
                    prev_item = current_item;
                    current_item = current_item->next;
                }
            }

            // �浹 ó��: �ⱸ
            if (g->player->pos.x == g->exit->pos.x && g->player->pos.y == g->exit->pos.y) {
                g->game_state = LevelUp;
            }
        }

        // 3. ���� ���� ���� Ȯ��
        if (g->game_state == GameOver || g->game_state == LevelUp) {
            should_break = 1;
        }

        // ---[ũ��Ƽ�� ���� ����]---
        // ��� ���� ������ ������ �������Ƿ� ����� �����մϴ�.
        lock_write_unlock(&g->lock);

        // ������ Ż���ؾ� �Ѵٸ� break
        if (should_break) {
            break;
        }

        // ����� ������ ���¿��� sleep�� ȣ���մϴ�.
        // �� �ð� ���� �ٸ� ������(draw_thread)�� ��Ȱ�ϰ� ������ �� �ֽ��ϴ�.
        mysleep(16); // �� 60 FPS
    }

    // ���� ������ ����� ��, �ٸ� ������鵵 Ȯ���� ����ǵ��� ���¸� �����մϴ�.
    lock_write_lock(&g->lock);
    if (g->game_state != LevelUp) {
        g->game_state = GameOver;
    }
    lock_write_unlock(&g->lock);

    return NULL;
}

void* draw_thread_func(void* argument) {
    Global* g = (Global*)argument;

    while (1) {
        Game_state current_state;

        lock_read_lock(&g->lock);
        current_state = g->game_state;

        if (current_state == GameOver || current_state == LevelUp) {
            lock_read_unlock(&g->lock);
            break;
        }

        if (current_state == GameRun) {
            clear_back_buffer(g); // 1. ����� ����
            // 2. ����ۿ� ��� ��� �׸���
            draw_maze(g);
            draw_items(g);
            draw_exit(g);
            draw_monsters(g);
            draw_user(g);
        }
        lock_read_unlock(&g->lock);

        // 3. ������ (�� �ܺο��� ����)
        if (current_state == GameRun) {
            render_frame(g);
        }
        else if (current_state == GameStop) {
            draw_game_drop(g); // GameStop�� ��ü ȭ���� �����Ƿ� ���� �׸�
        }

        mysleep(33); // �� 30 FPS
    }
    return NULL;
}