#include "platform_abstraction.h"
#include "mystructs.h"
#include "myfunc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    terminal_initial();

    Global* g = game_setup(1);
    if (g == NULL) {
        printf("게임 초기화에 실패했습니다.\n");
        terminal_exit();
        return 1;
    }

    clear_screen();

    int x, y;
    console_size(&y, &x);
    gotoxy(x / 2 - 10, y / 2);
    hide_cursor();

    // F11 감지
    printf("F11키를 눌러 전체화면으로 전환해 주세요.\n");
    while (1) {
#ifdef _WIN32
        if (GetAsyncKeyState(VK_F11) & 0x8000) break;
        mysleep(50);
#else
        int ch = mygetch();
        if (ch == KEY_F(11)) {
            mysleep(50);//여기서 충돌남 c기본 라이브러리랑 
            break;
        }
        //printf("[DEBUG] game_setup: int ch = %d\n", ch);
#endif
    }

    clear_screen();
    console_size(&y, &x);
    g->console_size.x = x;
    g->console_size.y = y;


    if (!setup_buffers(g)) {
        printf("화면 버퍼 생성에 실패했습니다.\n");
        cleanup_game(g);
        free(g);
        terminal_exit();
        return 1;
    }

    gotoxy(x / 2 - 10, y / 2);
    printf("캐릭터 이름 입력: ");
    show_cursor(); // 이름 입력을 위해 커서 보이기
    fgets(g->player->name, sizeof(g->player->name), stdin);
    g->player->name[strcspn(g->player->name, "\n")] = 0;
    hide_cursor(); // 입력 후 다시 숨기기

    clear_screen();

    thread_data Key_deliver_Thread, Game_logic_Thread, Draw_Thread;
    if (thread_create(&Draw_Thread, draw_thread_func, g) != 0) {
        printf("출력스레드 생성 실패");
        cleanup_game(g);
        free(g);
        terminal_exit();
        return 1;
    }
    if (thread_create(&Game_logic_Thread, game_logic_thread_func, g) != 0) {
        printf("로직스레드 생성 실패");
        g->game_state = GameOver;
        thread_join(Draw_Thread);
        cleanup_game(g);
        free(g);
        terminal_exit();
        return 1;
    }
    if (thread_create(&Key_deliver_Thread, key_deliver_thread_func, g) != 0) {
        printf("키전달스레드 생성 실패");
        g->game_state = GameOver;
        thread_join(Draw_Thread);
        thread_join(Game_logic_Thread);
        cleanup_game(g);
        free(g);
        terminal_exit();
        return 1;
    }

    thread_join(Key_deliver_Thread);
    thread_join(Game_logic_Thread);
    thread_join(Draw_Thread);

    cleanup_game(g);
    free(g);

    clear_screen();
    gotoxy(y, x);
    printf("게임이 종료되었습니다. 이용해주셔서 감사합니다.\n");
    show_cursor();
    terminal_exit();

    //실행 코드
    system("pause"); //반드시 return 0; 위에 추가
    //시간관계상 windows것만
    return 0;
}