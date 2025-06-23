#include "platform_abstraction.h"
#include "mystructs.h"
#include "myfunc.h"
// #include <stdio.h>
#include <stdlib.h>
// #include <time.h>
#include <string.h>

//미로벽정보 생성 함수
Cell** make_cell(int level) {
    // srand((unsigned int)time(NULL));
    int i;
    int row_cnt = level * 50;
    int col_cnt = level * 50;

    Cell** maze_cell = (Cell**)malloc((sizeof(Cell*) * row_cnt)); //행열
    if (maze_cell == NULL) { return NULL; }
    Cell** grop = (Cell**)malloc(sizeof(Cell*) * col_cnt); //마지막행이 아닐때 바닥뚫을때 쓸 배열
    if (grop == NULL) { exit(EXIT_FAILURE); }

    int set = 0;

    for (i = 0; i < row_cnt; i++) { //행반복
        maze_cell[i] = (Cell*)malloc(sizeof(Cell) * col_cnt); //행 만들어줌
        if (maze_cell[i] == NULL) { goto maze_cell_clean; }

        for (int j = 0; j < col_cnt; j++) { //열을 만들고 숫자를 넣어줌
            maze_cell[i][j].top = 1; //천장막음
            maze_cell[i][j].bottom = 1;//바닥막음
            maze_cell[i][j].left = 1; //좌측 벽 막음
            maze_cell[i][j].right = 1; //우측 벽 막음
            //집합 번호 넣기

            if (i > 0 && maze_cell[i - 1][j].bottom == 0) {// 첫쨰행이 아니거나 위층의 바닥이 없다면
                maze_cell[i][j].top = 0;//천장 뚫음
                maze_cell[i][j].num = maze_cell[i - 1][j].num; //위의 숫자 받음
            }
            else {
                //아니면
                maze_cell[i][j].num = set++;
            }
        }

        if (i != row_cnt - 1) { //마지막 행이 아닐때
            for (int j = 0; j < col_cnt - 1; j++) { //벽 허물기
                if (maze_cell[i][j].num != maze_cell[i][j + 1].num) { //같은 숫자가 아니면
                    maze_cell[i][j].right = rand() % 2;//50%허묾
                }
                else {//같은숫자면
                    maze_cell[i][j].right = 0;//허물어줌
                }

                if (maze_cell[i][j].right == 0 && j != col_cnt - 1) { //벽을 허물었다면
                    maze_cell[i][j + 1].left = 0; //반대편도 허물어줌

                    int a = maze_cell[i][j + 1].num;
                    for (int k = 0; k < col_cnt; k++) { // 전체 돌음
                        if (maze_cell[i][k].num == a) {// 이 번호와 같은 숫자들
                            maze_cell[i][k].num = maze_cell[i][j].num;// 모두 바꿔줌
                        }
                    }
                }
            }

            int g_len = 0;//길이
            //int b_cnt = 0;//바닥값

            for (int j = 0; j < col_cnt; j++) {
                //바닥 뚫기
                grop[g_len] = &maze_cell[i][j];
                g_len++;
                //우측 끝이거나 현재 번호와 이후 번호가 다를때
                if (j == col_cnt - 1 || (maze_cell[i][j].num != maze_cell[i][j + 1].num)) {

                    int bottom_check = 0; // 바닥체크

                    for (int k = 0; k < g_len; k++) { //그룹중 랜덤 바닥 뚫기
                        if (rand() % 4 == 0) {//25%확률
                            grop[k]->bottom = 0;
                            bottom_check = 1;//최소 하나의 바닥이 뚫림
                        }
                    }
                    //bottom_check++ 안됨... 안전장치 먹통됨...
                    if (bottom_check == 0) {
                        int a = rand() % g_len;
                        grop[a]->bottom = 0; // 강제로 하나 뚫는다.
                    }

                    g_len = 0;
                }
            }

        }
        else {// 마지막행일때
            for (int j = 0; j < col_cnt - 1; j++) {
                if (maze_cell[i][j].num != maze_cell[i][j + 1].num) {// 같은 숫자가 아니면
                    maze_cell[i][j].right = 0;// 벽을 부심
                    maze_cell[i][j + 1].left = 0;

                    int a = maze_cell[i][j + 1].num;

                    for (int k = 0; k < col_cnt; k++) { //전체 돌음
                        if (maze_cell[i][k].num == a) {// 이 번호와 같은 숫자들
                            maze_cell[i][k].num = maze_cell[i][j].num;// 모두 바꿔줌
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
} // 미로 구조 생성


//미로맵 구현 함수
Maze** make_maze(int level) {
    Cell** cell = make_cell(level);

    int i;
    int col = (level * 50) * 2 + 1;
    int row = (level * 50) * 2 + 1;
    Maze** maze = (Maze**)malloc((sizeof(Maze*) * row)); //행열
    if (maze == NULL) return NULL;

    for (i = 0; i < row; i++) { //행반복
        maze[i] = (Maze*)malloc(sizeof(Maze) * col); //행 만들어줌
        if (maze[i] == NULL) goto maze_clean;
        for (int j = 0; j < col; j++) {//열 만들어줌
            maze[i][j].cell = 1;
            maze[i][j].monster = 0;
            maze[i][j].item = 0;
            maze[i][j].exit = 0;
        }
    }
    for (int i = 0; i < row; i++) { //행반복
        for (int j = 0; j < col; j++) { //열을 만들고 숫자를 넣어줌
            if (j % 2 == 1 && i % 2 == 1) {//홀수 행열
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
            }//if 홀수 행열
        }//for열
    }//for행

    for (int j = 0; j < level * 50; j++) {
        free(cell[j]);//토사구팽
    }
    free(cell);

    return maze;

maze_clean:
    for (int a = 0; a < i; a++) {
        free(maze[a]);
    }
    free(maze);
    // cell 배열도 해제
    for (int j = 0; j < level * 50; j++) {
        free(cell[j]);
    }

} // 구조만들고 맵을 그림