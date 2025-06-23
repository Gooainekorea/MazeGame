#pragma once
// #include "platform_abstraction.h"
#include "mystructs.h"
// #include <stdio.h>
// #include <stdbool.h>

//maze
Maze** make_maze(int level);


// game_manager.c
Global* game_setup(int level);//�ʱ�ȭ
void cleanup_game(Global* g);//ġ���
void create_monsters(Global* g);//���ͻ���
void create_item(Item** h, Global* g);//�����ۻ���
void create_exit(Global* g);//�ⱸ����
void delete_node(Item* node, Global* g);//������
void freeList(Global* g);//���Ḯ��Ʈ����
void move_maze(Global* g, int direction);//�̷� ������
void move_element(Global* g, Move_element* element);//��� ������
int setup_buffers(Global* g);


// draw_func.c
void render_frame(Global* g);
void clear_back_buffer(Global* g);
//�׸�
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