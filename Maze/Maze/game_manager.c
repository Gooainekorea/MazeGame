#include "platform_abstraction.h"
#include "mystructs.h"
#include "myfunc.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// 게임 자원 해제 함수
void cleanup_game(Global* g) {
    if (!g) return;

    // 할당의 역순으로 해제하는 것이 일반적이고 안전합니다.
    // 아 그래서 쓰레드도 역순으로 해제하는구나
    // 
    // 버퍼 해제

    free(g->front_buffer->cells);
    free(g->front_buffer);
    free(g->back_buffer->cells);
    free(g->back_buffer);

    // 미로 해제
    if (g->maze) {
        for (int i = 0; i < g->maze_size; i++) {
            free(g->maze[i]); // 각 행을 먼저 해제
        }
        free(g->maze); // 행 포인터 배열을 해제
        g->maze = NULL; // Dangling pointer 방지
    }

    // 2. 다른 동적 배열 및 구조체 해제
    free(g->player);
    free(g->monsters);
    free(g->exit);
    freeList(g);


    // 3. 락(Lock) 자원 해제
    lock_destroy(&g->lock);
}

// 게임 초기화 함수
Global* game_setup(int level) {
    Global* g = (Global*)malloc(sizeof(Global));
    if (g == NULL) {
        printf("[DEBUG] game_setup: Global 구조체 메모리 할당 실패!\n");
        return NULL;
    }
    //printf("[DEBUG] game_setup: Global 구조체 메모리 할당 완료.\n");

        //printf("[DEBUG] game_setup: 함수 시작.\n");
    srand((unsigned int)time(NULL));
    //printf("[DEBUG] game_setup: time 시드 설정.\n");
// 레벨 기반 값 설정
    g->level = level;
    g->maze_size = (g->level * 50) * 2 + 1;
    g->monster_cnt = (rand() % (g->level * 2)) + 3;
    g->item_cnt = (rand() % (g->level * 2)) + 3;
    g->game_state = GameRun;
    g->key = 0;
    //printf("[DEBUG] game_setup: 데이터 값 넣음.\n");

// 2. 모든 포인터 멤버 메모리 할당
    g->player = (User*)malloc(sizeof(User));
    g->exit = (Move_element*)malloc(sizeof(Move_element)); // Exit 타입을 Move_element로 가정
    g->monsters = (Monster*)malloc(sizeof(Monster) * g->monster_cnt);
    g->item_list_h = NULL;
    g->maze = make_maze(g->level); // make_maze는 내부적으로 malloc을 하므로 여기서 호출
    //아 ㄴ여기서 안바꿔줘서 사이즈가 이상해진거였구나
    //printf("[DEBUG] game_setup: 구조체 값 넣음.\n");

// 3. 할당 실패 검사
    if (!g->player || !g->exit || !g->monsters || !g->maze) {
        printf("전역구조체 할당실패.\n");
        // 실패 시, 이미 할당된 것들을 모두 해제해줘야 함
        free(g->player);
        free(g->exit);
        free(g->monsters);
        if (g->maze) {
            for (int i = 0; i < g->maze_size; i++) free(g->maze[i]);
            free(g->maze);
        }
        freeList(g);
        free(g);
        return NULL;
    }
    // 버퍼 포인터 초기화
    g->front_buffer = NULL;
    g->back_buffer = NULL;
    // 미할당된 메모리의 내용 채우기(임시)
    g->console_size.y = 1;
    g->console_size.x = 1;
    //미로 가운데에서 시작
    g->player->pos.x = g->maze_size / 2;
    g->player->pos.y = g->maze_size / 2;
    while (g->player->pos.y < g->maze_size - 1 && g->maze[g->player->pos.y][g->player->pos.x].cell != 0) {
        g->player->pos.y++;
    }

    g->player->hp = 100;
    //printf("[DEBUG] game_setup: 미할당된 메모리 값 넣음.\n");
    create_monsters(g);
    create_exit(g);
    create_item(&g->item_list_h, g);
    //printf("[DEBUG] game_setup: 몬스터/아이템 배열 채우는 함수 호출.\n");
// 5. 잠금 초기화 (모든 할당이 끝난 후 마지막에)
    if (lock_init(&g->lock) != 0) {
        printf("잠금 초기화 실패 \n");
        // cleanup_game(g)을 호출하여 모든 리소스를 해제하는 것이 좋음
        cleanup_game(g);
        return NULL;
    }
    //printf("[DEBUG] game_setup: render_lock 잠금 해제 성공.\n");
    return g;
}
// 버퍼 초기화 함수
int setup_buffers(Global* g) {
    // 버퍼 크기는 콘솔 크기에 맞춤
    int width = g->console_size.x;
    int height = g->console_size.y;

    // wide character(한글, 특수문자)가 2칸을 차지하므로 버퍼의 가로 길이는 절반으로 함
    // 하지만 렌더링 시 x*2를 해주므로, 버퍼의 width는 콘솔의 셀 개수와 동일하게 가는 것이 직관적
    width = g->console_size.x / 2;

    g->front_buffer = (ScreenBuffer*)malloc(sizeof(ScreenBuffer));
    g->back_buffer = (ScreenBuffer*)malloc(sizeof(ScreenBuffer));

    if (!g->front_buffer || !g->back_buffer) return 0;

    g->front_buffer->width = g->back_buffer->width = width;
    g->front_buffer->height = g->back_buffer->height = height;

    g->front_buffer->cells = (ScreenCell*)malloc(sizeof(ScreenCell) * width * height);
    g->back_buffer->cells = (ScreenCell*)malloc(sizeof(ScreenCell) * width * height);

    if (!g->front_buffer->cells || !g->back_buffer->cells) return 0;

    // 버퍼 초기화
    for (int i = 0; i < width * height; i++) {
        g->front_buffer->cells[i] = -1; // 첫 프레임에서 강제로 그리도록 다른 값으로 초기화
        g->back_buffer->cells[i] = C_null;
    }

    return 1;
}


static Position random_empty_pos(Global* g) {
    Position pos;
    while (1) {
        pos.x = rand() % g->maze_size;
        pos.y = rand() % g->maze_size;
        if ((pos.y < g->maze_size - 1 && pos.x < g->maze_size - 1) && g->maze[pos.y][pos.x].cell == 0) {
            break;
        }
    }
    return pos;
}

void create_monsters(Global* g) {
    //g->monsters = (Monster*)malloc(sizeof(Monster) * g->monster_cnt);
    if (g->monsters == NULL) { return; }

    for (int i = 0; i < g->monster_cnt; i++) {
        Position a = random_empty_pos(g);
        g->monsters[i].num = i;
        g->monsters[i].move.pos = a;
        g->monsters[i].move.speed = rand() % 4 + 1;
        g->monsters[i].move.direction = rand() % 4;
        g->monsters[i].move.distance = rand() % 10 + 5;
        g->monsters[i].attack = rand() % g->level * 5;
        g->monsters[i].move.last_move_time = get_current_time_ms() - (rand() % 500);
        g->maze[a.y][a.x].monster = 1;
    }

}

//포인터로 받았는데.... 생각해보니까 자체가 일반변수가 아니라 포인터 변수여서 한번 더싸매야했어
void create_item(Item** h, Global* g) {//처음 생성
    for (int i = 0; i < g->item_cnt; i++) {
        Position a = random_empty_pos(g);
        Item* item_list_h = (Item*)malloc(sizeof(Item));
        item_list_h->pos = a;
        item_list_h->id = i;
        item_list_h->hp_recvr = rand() % g->level + 10;
        item_list_h->next = *h;
        *h = item_list_h;
        g->maze[a.y][a.x].item = 1;
    }

}
Item* search_node(Item* node, Global* g) {//검색 - 쓸일이 있을까.
    Item* temp = g->item_list_h;
    while (temp != NULL) {
        if (temp->id == node->id) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}
void delete_node(Item* node, Global* g) {//삭제
    Item* temp = g->item_list_h;
    Item* prev = NULL;

    // if (temp == NULL || node == NULL) { return; }
    if (temp == node) {
        g->item_list_h = temp->next;
        free(node);
        return;
    }
    while (temp != NULL && temp != node) {
        prev = temp;
        temp = temp->next;
    }
    if (temp) {
        prev->next = node->next;
        free(node);
    }

}
void freeList(Global* g) {//해제
    Item* temp;
    while (g->item_list_h != NULL) {
        temp = g->item_list_h;
        g->item_list_h = g->item_list_h->next;
        free(temp);

    }
}

void create_exit(Global* g) {
    Position a = random_empty_pos(g);
    if (g->exit == NULL) { return; }
    g->exit->pos = a;
    g->exit->speed = 1;
    g->exit->direction = rand() % 4;
    g->exit->distance = rand() % 10 + 5;
    g->exit->last_move_time = get_current_time_ms() - (rand() % 500);;
    g->maze[a.y][a.x].exit = 1;

}