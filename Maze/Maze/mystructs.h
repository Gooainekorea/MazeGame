#pragma once
#include "platform_abstraction.h"

// ScreenCell: 화면의 각 셀이 나타내는 요소를 정의
// 버퍼링을 위해 빈 공간(CELL_EMPTY)과 플레이어(CELL_PLAYER)를 추가
typedef enum {
    C_null,   // 빈 공간- 특수문자 그대로 썼는데 오류남
    C_wall,    // 벽 ■
    C_monster, // 몬스터 ※
    C_item,    // 아이템 ♬
    C_exit,    // 출구 ★
    C_user   // 플레이어 ●
} ScreenCell;

typedef enum {
    GameOver, GameRun, GameStop, LevelUp
} Game_state;

typedef struct { // 미로 Cell (기존 구조체 유지)
    int num;
    int top : 1, bottom : 1, left : 1, right : 1;
} Cell;

typedef struct { // 미로 데이터 (기존 구조체 유지)
    int cell : 1;
    int monster : 1;
    int item : 1;
    int exit : 1;
} Maze;

typedef struct { // 위치
    int x;
    int y;
} Position;

typedef struct { // 유저
    char name[10];
    Position pos;
    int hp;
    int d_buff;
    int st;
} User;

typedef struct { // 움직이는 요소
    Position pos;
    int speed;
    int direction;
    int distance;
    long last_move_time;
} Move_element;

typedef struct { // 몬스터
    int num;
    Move_element move;
    int attack;
    int d_buff;
} Monster;

typedef struct Item { // 아이템 (연결 리스트)
    int id;
    Position pos;
    int hp_recvr;
    int dbuf_remval;
    struct Item* next;
} Item;

// 화면 전체를 담는 버퍼 구조체 (수정됨)
typedef struct {
    ScreenCell* cells; // 화면 셀 데이터를 담는 1차원 배열
    int width;
    int height;
} ScreenBuffer;

// 전역 구조체 (수정됨)
typedef struct {
    // 데이터 포인터들
    Maze** maze;
    User* player;
    Monster* monsters;
    Item* item_list_h;
    Move_element* exit;

    // 데이터 값들
    int maze_size;
    int monster_cnt;
    int item_cnt;
    Position console_size; // 콘솔 크기 (문자 셀 기준)

    int level;
    int key;
    Game_state game_state;

    // 더블 버퍼링을 위한 버퍼 포인터
    ScreenBuffer* front_buffer; // 현재 화면에 그려진 상태
    ScreenBuffer* back_buffer;  // 새로 그릴 상태를 담는 버퍼

    lock_data lock; // 동기화 락
} Global;