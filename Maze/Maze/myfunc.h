#pragma once
// #include "platform_abstraction.h"
#include "mystructs.h"
// #include <stdio.h>
// #include <stdbool.h>

//maze
Maze** make_maze(int level);


// game_manager.c
Global* game_setup(int level);//초기화
void cleanup_game(Global* g);//치우기
void create_monsters(Global* g);//몬스터생성
void create_item(Item** h, Global* g);//아이템생성
void create_exit(Global* g);//출구생성
void delete_node(Item* node, Global* g);//노드삭제
void freeList(Global* g);//연결리스트해제
void move_maze(Global* g, int direction);//미로 움직임
void move_element(Global* g, Move_element* element);//요소 움직임
int setup_buffers(Global* g);


// draw_func.c
void render_frame(Global* g);
void clear_back_buffer(Global* g);
//그림
void draw_maze(Global* g);
void draw_monsters(Global* g);
void draw_items(Global* g);
void draw_exit(Global* g);
void draw_user(Global* g);
void draw_ui(Global* g);
void draw_game_drop(Global* g);


// thread_func.c
void* key_deliver_thread_func(void* argument);
void* game_logic_thread_func(void* argument);
void* draw_thread_func(void* argument);