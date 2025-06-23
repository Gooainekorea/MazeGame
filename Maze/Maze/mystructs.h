#pragma once
#include "platform_abstraction.h"

// ScreenCell: ȭ���� �� ���� ��Ÿ���� ��Ҹ� ����
// ���۸��� ���� �� ����(CELL_EMPTY)�� �÷��̾�(CELL_PLAYER)�� �߰�
typedef enum {
    C_null,   // �� ����- Ư������ �״�� ��µ� ������
    C_wall,    // �� ��
    C_monster, // ���� ��
    C_item,    // ������ ��
    C_exit,    // �ⱸ ��
    C_user   // �÷��̾� ��
} ScreenCell;

typedef enum {
    GameOver, GameRun, GameStop, LevelUp
} Game_state;

typedef struct { // �̷� Cell (���� ����ü ����)
    int num;
    int top : 1, bottom : 1, left : 1, right : 1;
} Cell;

typedef struct { // �̷� ������ (���� ����ü ����)
    int cell : 1;
    int monster : 1;
    int item : 1;
    int exit : 1;
} Maze;

typedef struct { // ��ġ
    int x;
    int y;
} Position;

typedef struct { // ����
    char name[10];
    Position pos;
    int hp;
    int d_buff;
    int st;
} User;

typedef struct { // �����̴� ���
    Position pos;
    int speed;
    int direction;
    int distance;
    long last_move_time;
} Move_element;

typedef struct { // ����
    int num;
    Move_element move;
    int attack;
    int d_buff;
} Monster;

typedef struct Item { // ������ (���� ����Ʈ)
    int id;
    Position pos;
    int hp_recvr;
    int dbuf_remval;
    struct Item* next;
} Item;

// ȭ�� ��ü�� ��� ���� ����ü (������)
typedef struct {
    ScreenCell* cells; // ȭ�� �� �����͸� ��� 1���� �迭
    int width;
    int height;
} ScreenBuffer;

// ���� ����ü (������)
typedef struct {
    // ������ �����͵�
    Maze** maze;
    User* player;
    Monster* monsters;
    Item* item_list_h;
    Move_element* exit;

    // ������ ����
    int maze_size;
    int monster_cnt;
    int item_cnt;
    Position console_size; // �ܼ� ũ�� (���� �� ����)

    int level;
    int key;
    Game_state game_state;

    // ���� ���۸��� ���� ���� ������
    ScreenBuffer* front_buffer; // ���� ȭ�鿡 �׷��� ����
    ScreenBuffer* back_buffer;  // ���� �׸� ���¸� ��� ����

    lock_data lock; // ����ȭ ��
} Global;