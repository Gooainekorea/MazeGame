#include "platform_abstraction.h"
#include "mystructs.h"
#include "myfunc.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ���� �ڿ� ���� �Լ�
void cleanup_game(Global* g) {
    if (!g) return;

    // �Ҵ��� �������� �����ϴ� ���� �Ϲ����̰� �����մϴ�.
    // �� �׷��� �����嵵 �������� �����ϴ±���
    // 
    // ���� ����

    free(g->front_buffer->cells);
    free(g->front_buffer);
    free(g->back_buffer->cells);
    free(g->back_buffer);

    // �̷� ����
    if (g->maze) {
        for (int i = 0; i < g->maze_size; i++) {
            free(g->maze[i]); // �� ���� ���� ����
        }
        free(g->maze); // �� ������ �迭�� ����
        g->maze = NULL; // Dangling pointer ����
    }

    // 2. �ٸ� ���� �迭 �� ����ü ����
    free(g->player);
    free(g->monsters);
    free(g->exit);
    freeList(g);


    // 3. ��(Lock) �ڿ� ����
    lock_destroy(&g->lock);
}

// ���� �ʱ�ȭ �Լ�
Global* game_setup(int level) {
    Global* g = (Global*)malloc(sizeof(Global));
    if (g == NULL) {
        printf("[DEBUG] game_setup: Global ����ü �޸� �Ҵ� ����!\n");
        return NULL;
    }
    //printf("[DEBUG] game_setup: Global ����ü �޸� �Ҵ� �Ϸ�.\n");

        //printf("[DEBUG] game_setup: �Լ� ����.\n");
    srand((unsigned int)time(NULL));
    //printf("[DEBUG] game_setup: time �õ� ����.\n");
// ���� ��� �� ����
    g->level = level;
    g->maze_size = (g->level * 50) * 2 + 1;
    g->monster_cnt = (rand() % (g->level * 2)) + 3;
    g->item_cnt = (rand() % (g->level * 2)) + 3;
    g->game_state = GameRun;
    g->key = 0;
    //printf("[DEBUG] game_setup: ������ �� ����.\n");

// 2. ��� ������ ��� �޸� �Ҵ�
    g->player = (User*)malloc(sizeof(User));
    g->exit = (Move_element*)malloc(sizeof(Move_element)); // Exit Ÿ���� Move_element�� ����
    g->monsters = (Monster*)malloc(sizeof(Monster) * g->monster_cnt);
    g->item_list_h = NULL;
    g->maze = make_maze(g->level); // make_maze�� ���������� malloc�� �ϹǷ� ���⼭ ȣ��
    //�� �����⼭ �ȹٲ��༭ ����� �̻������ſ�����
    //printf("[DEBUG] game_setup: ����ü �� ����.\n");

// 3. �Ҵ� ���� �˻�
    if (!g->player || !g->exit || !g->monsters || !g->maze) {
        printf("��������ü �Ҵ����.\n");
        // ���� ��, �̹� �Ҵ�� �͵��� ��� ��������� ��
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
    // ���� ������ �ʱ�ȭ
    g->front_buffer = NULL;
    g->back_buffer = NULL;
    // ���Ҵ�� �޸��� ���� ä���(�ӽ�)
    g->console_size.y = 1;
    g->console_size.x = 1;
    //�̷� ������� ����
    g->player->pos.x = g->maze_size / 2;
    g->player->pos.y = g->maze_size / 2;
    while (g->player->pos.y < g->maze_size - 1 && g->maze[g->player->pos.y][g->player->pos.x].cell != 0) {
        g->player->pos.y++;
    }

    g->player->hp = 100;
    //printf("[DEBUG] game_setup: ���Ҵ�� �޸� �� ����.\n");
    create_monsters(g);
    create_exit(g);
    create_item(&g->item_list_h, g);
    //printf("[DEBUG] game_setup: ����/������ �迭 ä��� �Լ� ȣ��.\n");
// 5. ��� �ʱ�ȭ (��� �Ҵ��� ���� �� ��������)
    if (lock_init(&g->lock) != 0) {
        printf("��� �ʱ�ȭ ���� \n");
        // cleanup_game(g)�� ȣ���Ͽ� ��� ���ҽ��� �����ϴ� ���� ����
        cleanup_game(g);
        return NULL;
    }
    //printf("[DEBUG] game_setup: render_lock ��� ���� ����.\n");
    return g;
}
// ���� �ʱ�ȭ �Լ�
int setup_buffers(Global* g) {
    // ���� ũ��� �ܼ� ũ�⿡ ����
    int width = g->console_size.x;
    int height = g->console_size.y;

    // wide character(�ѱ�, Ư������)�� 2ĭ�� �����ϹǷ� ������ ���� ���̴� �������� ��
    // ������ ������ �� x*2�� ���ֹǷ�, ������ width�� �ܼ��� �� ������ �����ϰ� ���� ���� ������
    width = g->console_size.x / 2;

    g->front_buffer = (ScreenBuffer*)malloc(sizeof(ScreenBuffer));
    g->back_buffer = (ScreenBuffer*)malloc(sizeof(ScreenBuffer));

    if (!g->front_buffer || !g->back_buffer) return 0;

    g->front_buffer->width = g->back_buffer->width = width;
    g->front_buffer->height = g->back_buffer->height = height;

    g->front_buffer->cells = (ScreenCell*)malloc(sizeof(ScreenCell) * width * height);
    g->back_buffer->cells = (ScreenCell*)malloc(sizeof(ScreenCell) * width * height);

    if (!g->front_buffer->cells || !g->back_buffer->cells) return 0;

    // ���� �ʱ�ȭ
    for (int i = 0; i < width * height; i++) {
        g->front_buffer->cells[i] = -1; // ù �����ӿ��� ������ �׸����� �ٸ� ������ �ʱ�ȭ
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

//�����ͷ� �޾Ҵµ�.... �����غ��ϱ� ��ü�� �Ϲݺ����� �ƴ϶� ������ �������� �ѹ� ���θž��߾�
void create_item(Item** h, Global* g) {//ó�� ����
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
Item* search_node(Item* node, Global* g) {//�˻� - ������ ������.
    Item* temp = g->item_list_h;
    while (temp != NULL) {
        if (temp->id == node->id) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}
void delete_node(Item* node, Global* g) {//����
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
void freeList(Global* g) {//����
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