#include "platform_abstraction.h"
#include "mystructs.h"
#include "myfunc.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

// 키 전달 스레드 (수정 불필요, 기존 코드 양호)
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

// 쓰레드 구현 이런거 하지 않는게 정신에 이롭다
void* game_logic_thread_func(void* argument) {
    Global* g = (Global*)argument;
    while (1) {
        int should_break = 0;

        // 공유 데이터(g)를 읽고 쓰는 모든 로직을 하나의 잠금 블록 안에서 처리합니다.
        // 이를 통해 데이터의 일관성을 보장합니다.
        lock_write_lock(&g->lock);

        // 현재 상태와 처리할 키를 지역 변수로 가져옵니다.
        Game_state current_state = g->game_state;
        int key_to_process = g->key;

        // 키를 처리했으므로 공유 변수를 0으로 리셋합니다.
        if (key_to_process != 0) {
            g->key = 0;
        }

        // 1. 상태에 따른 키 입력 처리
        if (current_state == GameRun) {
            if (key_to_process == 27) { // ESC 키
                g->game_state = GameStop;
            }

            // 방향키 처리 (플레이어 이동)
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

        // 2. 게임 로직 업데이트 (GameRun 상태일 때만)
        if (g->game_state == GameRun) {
            long long current_time = get_current_time_ms();

            // 몬스터 이동
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

            // 출구 이동
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

            // 충돌 처리: 몬스터
            for (int i = 0; i < g->monster_cnt; i++) {
                if (g->player->pos.x == g->monsters[i].move.pos.x && g->player->pos.y == g->monsters[i].move.pos.y) {
                    g->player->hp -= g->monsters[i].attack;
                    if (g->player->hp <= 0) {
                        g->game_state = GameOver;
                    }
                }
            }

            // 충돌 처리: 아이템
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

            // 충돌 처리: 출구
            if (g->player->pos.x == g->exit->pos.x && g->player->pos.y == g->exit->pos.y) {
                g->game_state = LevelUp;
            }
        }

        // 3. 루프 종료 조건 확인
        if (g->game_state == GameOver || g->game_state == LevelUp) {
            should_break = 1;
        }

        // ---[크리티컬 섹션 종료]---
        // 모든 공유 데이터 접근이 끝났으므로 잠금을 해제합니다.
        lock_write_unlock(&g->lock);

        // 루프를 탈출해야 한다면 break
        if (should_break) {
            break;
        }

        // 잠금이 해제된 상태에서 sleep을 호출합니다.
        // 이 시간 동안 다른 스레드(draw_thread)가 원활하게 동작할 수 있습니다.
        mysleep(16); // 약 60 FPS
    }

    // 게임 루프가 종료된 후, 다른 스레드들도 확실히 종료되도록 상태를 전파합니다.
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
            clear_back_buffer(g); // 1. 백버퍼 비우기
            // 2. 백버퍼에 모든 요소 그리기
            draw_maze(g);
            draw_items(g);
            draw_exit(g);
            draw_monsters(g);
            draw_user(g);
        }
        lock_read_unlock(&g->lock);

        // 3. 렌더링 (락 외부에서 수행)
        if (current_state == GameRun) {
            render_frame(g);
        }
        else if (current_state == GameStop) {
            draw_game_drop(g); // GameStop은 전체 화면을 덮으므로 직접 그림
        }

        mysleep(33); // 약 30 FPS
    }
    return NULL;

}
