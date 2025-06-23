#include "platform_abstraction.h"
#include "mystructs.h"
#include "myfunc.h"
// #include <stdio.h>
#include <stdlib.h>
// #include <time.h>
#include <string.h>

//�̷κ����� ���� �Լ�
Cell** make_cell(int level) {
    // srand((unsigned int)time(NULL));
    int i;
    int row_cnt = level * 50;
    int col_cnt = level * 50;

    Cell** maze_cell = (Cell**)malloc((sizeof(Cell*) * row_cnt)); //�࿭
    if (maze_cell == NULL) { return NULL; }
    Cell** grop = (Cell**)malloc(sizeof(Cell*) * col_cnt); //���������� �ƴҶ� �ٴڶ����� �� �迭
    if (grop == NULL) { exit(EXIT_FAILURE); }

    int set = 0;

    for (i = 0; i < row_cnt; i++) { //��ݺ�
        maze_cell[i] = (Cell*)malloc(sizeof(Cell) * col_cnt); //�� �������
        if (maze_cell[i] == NULL) { goto maze_cell_clean; }

        for (int j = 0; j < col_cnt; j++) { //���� ����� ���ڸ� �־���
            maze_cell[i][j].top = 1; //õ�帷��
            maze_cell[i][j].bottom = 1;//�ٴڸ���
            maze_cell[i][j].left = 1; //���� �� ����
            maze_cell[i][j].right = 1; //���� �� ����
            //���� ��ȣ �ֱ�

            if (i > 0 && maze_cell[i - 1][j].bottom == 0) {// ù������ �ƴϰų� ������ �ٴ��� ���ٸ�
                maze_cell[i][j].top = 0;//õ�� ����
                maze_cell[i][j].num = maze_cell[i - 1][j].num; //���� ���� ����
            }
            else {
                //�ƴϸ�
                maze_cell[i][j].num = set++;
            }
        }

        if (i != row_cnt - 1) { //������ ���� �ƴҶ�
            for (int j = 0; j < col_cnt - 1; j++) { //�� �㹰��
                if (maze_cell[i][j].num != maze_cell[i][j + 1].num) { //���� ���ڰ� �ƴϸ�
                    maze_cell[i][j].right = rand() % 2;//50%�㹲
                }
                else {//�������ڸ�
                    maze_cell[i][j].right = 0;//�㹰����
                }

                if (maze_cell[i][j].right == 0 && j != col_cnt - 1) { //���� �㹰���ٸ�
                    maze_cell[i][j + 1].left = 0; //�ݴ��� �㹰����

                    int a = maze_cell[i][j + 1].num;
                    for (int k = 0; k < col_cnt; k++) { // ��ü ����
                        if (maze_cell[i][k].num == a) {// �� ��ȣ�� ���� ���ڵ�
                            maze_cell[i][k].num = maze_cell[i][j].num;// ��� �ٲ���
                        }
                    }
                }
            }

            int g_len = 0;//����
            //int b_cnt = 0;//�ٴڰ�

            for (int j = 0; j < col_cnt; j++) {
                //�ٴ� �ձ�
                grop[g_len] = &maze_cell[i][j];
                g_len++;
                //���� ���̰ų� ���� ��ȣ�� ���� ��ȣ�� �ٸ���
                if (j == col_cnt - 1 || (maze_cell[i][j].num != maze_cell[i][j + 1].num)) {

                    int bottom_check = 0; // �ٴ�üũ

                    for (int k = 0; k < g_len; k++) { //�׷��� ���� �ٴ� �ձ�
                        if (rand() % 4 == 0) {//25%Ȯ��
                            grop[k]->bottom = 0;
                            bottom_check = 1;//�ּ� �ϳ��� �ٴ��� �ո�
                        }
                    }
                    //bottom_check++ �ȵ�... ������ġ �����...
                    if (bottom_check == 0) {
                        int a = rand() % g_len;
                        grop[a]->bottom = 0; // ������ �ϳ� �մ´�.
                    }

                    g_len = 0;
                }
            }

        }
        else {// ���������϶�
            for (int j = 0; j < col_cnt - 1; j++) {
                if (maze_cell[i][j].num != maze_cell[i][j + 1].num) {// ���� ���ڰ� �ƴϸ�
                    maze_cell[i][j].right = 0;// ���� �ν�
                    maze_cell[i][j + 1].left = 0;

                    int a = maze_cell[i][j + 1].num;

                    for (int k = 0; k < col_cnt; k++) { //��ü ����
                        if (maze_cell[i][k].num == a) {// �� ��ȣ�� ���� ���ڵ�
                            maze_cell[i][k].num = maze_cell[i][j].num;// ��� �ٲ���
                        }
                    }
                }
            }
        }
    }
    free(grop);
    return maze_cell;
maze_cell_clean:
    for (int cln = 0; cln < i; cln++) {
        free(maze_cell[cln]);
    }
    free(maze_cell);
    free(grop);
    return NULL;
} // �̷� ���� ����


//�̷θ� ���� �Լ�
Maze** make_maze(int level) {
    Cell** cell = make_cell(level);

    int i;
    int col = (level * 50) * 2 + 1;
    int row = (level * 50) * 2 + 1;
    Maze** maze = (Maze**)malloc((sizeof(Maze*) * row)); //�࿭
    if (maze == NULL) return NULL;

    for (i = 0; i < row; i++) { //��ݺ�
        maze[i] = (Maze*)malloc(sizeof(Maze) * col); //�� �������
        if (maze[i] == NULL) goto maze_clean;
        for (int j = 0; j < col; j++) {//�� �������
            maze[i][j].cell = 1;
            maze[i][j].monster = 0;
            maze[i][j].item = 0;
            maze[i][j].exit = 0;
        }
    }
    for (int i = 0; i < row; i++) { //��ݺ�
        for (int j = 0; j < col; j++) { //���� ����� ���ڸ� �־���
            if (j % 2 == 1 && i % 2 == 1) {//Ȧ�� �࿭
                int cell_y = i / 2;
                int cell_x = j / 2;
                maze[i][j].cell = 0;
                if (cell[cell_y][cell_x].top == 0) {
                    maze[i - 1][j].cell = 0;
                }
                if (cell[cell_y][cell_x].bottom == 0) {
                    maze[i + 1][j].cell = 0;
                }
                if (cell[cell_y][cell_x].left == 0) {
                    maze[i][j - 1].cell = 0;
                }
                if (cell[cell_y][cell_x].right == 0) {
                    maze[i][j + 1].cell = 0;
                }
            }//if Ȧ�� �࿭
        }//for��
    }//for��

    for (int j = 0; j < level * 50; j++) {
        free(cell[j]);//��籸��
    }
    free(cell);

    return maze;

maze_clean:
    for (int a = 0; a < i; a++) {
        free(maze[a]);
    }
    free(maze);
    // cell �迭�� ����
    for (int j = 0; j < level * 50; j++) {
        free(cell[j]);
    }

} // ��������� ���� �׸�