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
        printf("���� �ʱ�ȭ�� �����߽��ϴ�.\n");
        terminal_exit();
        return 1;
    }

    clear_screen();

    int x, y;
    console_size(&y, &x);
    gotoxy(x / 2 - 10, y / 2);
    hide_cursor();

    // F11 ����
    printf("F11Ű�� ���� ��üȭ������ ��ȯ�� �ּ���.\n");
    while (1) {
#ifdef _WIN32
        if (GetAsyncKeyState(VK_F11) & 0x8000) break;
        mysleep(50);
#else
        int ch = mygetch();
        if (ch == KEY_F(11)) {
            mysleep(50);//���⼭ �浹�� c�⺻ ���̺귯���� 
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
        printf("ȭ�� ���� ������ �����߽��ϴ�.\n");
        cleanup_game(g);
        free(g);
        terminal_exit();
        return 1;
    }

    gotoxy(x / 2 - 10, y / 2);
    printf("ĳ���� �̸� �Է�: ");
    show_cursor(); // �̸� �Է��� ���� Ŀ�� ���̱�
    fgets(g->player->name, sizeof(g->player->name), stdin);
    g->player->name[strcspn(g->player->name, "\n")] = 0;
    hide_cursor(); // �Է� �� �ٽ� �����

    clear_screen();

    thread_data Key_deliver_Thread, Game_logic_Thread, Draw_Thread;
    if (thread_create(&Draw_Thread, draw_thread_func, g) != 0) {
        printf("��½����� ���� ����");
        cleanup_game(g);
        free(g);
        terminal_exit();
        return 1;
    }
    if (thread_create(&Game_logic_Thread, game_logic_thread_func, g) != 0) {
        printf("���������� ���� ����");
        g->game_state = GameOver;
        thread_join(Draw_Thread);
        cleanup_game(g);
        free(g);
        terminal_exit();
        return 1;
    }
    if (thread_create(&Key_deliver_Thread, key_deliver_thread_func, g) != 0) {
        printf("Ű���޽����� ���� ����");
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
    printf("������ ����Ǿ����ϴ�. �̿����ּż� �����մϴ�.\n");
    show_cursor();
    terminal_exit();

    //���� �ڵ�
    system("pause"); //�ݵ�� return 0; ���� �߰�
    //�ð������ windows�͸�
    return 0;
}