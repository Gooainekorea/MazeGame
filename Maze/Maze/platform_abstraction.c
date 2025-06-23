#include "platform_abstraction.h"

#ifdef _WIN32 // Windows�� �Լ� tlqkf
void terminal_initial() {} //window������ ���� �͹̳� �ʱ�ȭ �ʿ� ������
void terminal_exit() {} //���ᵵ ���� �ʿ������
void gotoxy(int x, int y) {
    COORD pos = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}
void mysleep(int milliseconds) {
    Sleep(milliseconds);
}
int mykbhit() {
    return _kbhit();
}
int mygetch() {
    return _getch();
}
void console_size(int* rows, int* cols) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    *cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}
void hide_cursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE; // Ŀ���� bVisible ����� FALSE�� ����
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void show_cursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = TRUE; // Ŀ���� bVisible ����� TRUE�� ����
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}
//--------thread
int thread_create(thread_data* thread, thread_type start_routine, void* g_structs) {
    // Windows API ���
    *thread = CreateThread(
        NULL,                   // �⺻ ���� �Ӽ�
        0,                      // �⺻ ���� ũ��
        (LPTHREAD_START_ROUTINE)start_routine, // ������ �Լ�
        g_structs,                    // �Լ��� ������ ����
        0,                      // ��� ����
        NULL                    // ������ ID���� ��ġ- NULL : ���� ����
    );
    if (*thread == NULL) {
        return -1; // ����
    }
    return 0; // ����
}
int thread_join(thread_data thread) {
    // Windows API ���
    DWORD result = WaitForSingleObject(thread, INFINITE); //������ ������� ��� INFINITE : ���� ���
    CloseHandle(thread); // �ڵ� �ݱ�(���ҽ� ����)
    return (result == WAIT_OBJECT_0) ? 0 : -1;// 0: ����, 1: ����
}
//--------thread
//cls...
void clear_screen() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordScreen = { 0, 0 };    // Ȩ Ŀ�� ��ġ
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    // �ܼ� ȭ�� ���� ������ �����ɴϴ�.
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return;
    }

    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    // �ܼ� ȭ���� �������� ä��ϴ�.
    FillConsoleOutputCharacter(hConsole, (TCHAR)' ', dwConSize, coordScreen, &cCharsWritten);

    // �ܼ� ȭ���� �Ӽ�(���� ��)�� ���� �Ӽ����� �ٽ� ä��ϴ�.
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten);

    // Ŀ���� (0, 0)���� �̵���ŵ�ϴ�.
    SetConsoleCursorPosition(hConsole, coordScreen);
}//���� �̰� ���� ������?? cls���� �˾Ҵ���? cls ���������� ncurses�� �浹�����־ APIȣ���ؼ� ȭ�������ߴ���

//----------lock
int lock_init(lock_data* lock) {//�������
    InitializeSRWLock(lock);
    return 0;
}

int lock_destroy(lock_data* lock) {//��� ��
    (void)lock; // SRWLock�� ������� �ı� �Լ��� �ʿ� ����
    return 0;
}

int lock_read_lock(lock_data* lock) {//�б� ���
    AcquireSRWLockShared(lock);
    return 0;
}

int lock_read_unlock(lock_data* lock) {//�б� ��� ����
    ReleaseSRWLockShared(lock);
    return 0;
}

int lock_write_lock(lock_data* lock) {//���� ���
    AcquireSRWLockExclusive(lock);
    return 0;
}

int lock_write_unlock(lock_data* lock) {//���� ��� ����
    ReleaseSRWLockExclusive(lock);
    return 0;
}

//----------lock

long long get_current_time_ms() {
    return GetTickCount64();
}

#else // linux�� �Լ� Tlqkf

void terminal_initial() { //�͹̳� �ʱ�ȭ
    initscr();             // ncurses ��� ����
    cbreak();              // Line buffering ��Ȱ��ȭ
    noecho();              // �Էµ� Ű ����
    keypad(stdscr, TRUE);  // Ư�� Ű(F1, ȭ��ǥ ��) ��� ����
    curs_set(0);           // Ŀ�� ����
    nodelay(stdscr, TRUE); // getch()�� non-blocking���� ����
    mysleep(10);
}
void terminal_exit() { //�͹̳� ����
    endwin(); // ncurses ��� ����
}
void gotoxy(int x, int y) {
    move(y, x); // ncurses �Լ�
}
void mysleep(int milliseconds) {
    usleep(milliseconds * 1000); // usleep�� ����ũ���� ����
}
int mykbhit() {
    int ch = getch();
    if (ch != ERR) {
        ungetch(ch); // ���� ���ڸ� �ٽ� ���ۿ� ����
        return 1;
    }
    else {
        return 0;
    }
}
int mygetch() {
    return getch(); // nodelay�� �����Ǿ� �־� non-blocking���� ����
}
void console_size(int* rows, int* cols) {
    getmaxyx(stdscr, *rows, *cols);
}
void hide_cursor() {
    curs_set(0); // ncurses���� Ŀ���� ������ �ʰ� ����
}

void show_cursor() {
    curs_set(1); // ncurses���� Ŀ���� ���̰� ����
}
//--------thread
int thread_create(thread_data* thread, thread_type start_routine, void* g_structs) {
    return pthread_create(thread, NULL, start_routine, g_structs);
}
int thread_join(thread_data thread) {
    return pthread_join(thread, NULL);
}
//--------thread
void clear_screen() {
    clear();    // ncurses: ȭ�� ���۸� ����ϴ�.
    refresh();  // ncurses: ���� �͹̳ο� ���� ������ �����մϴ�.
}//�����츦 ����ϰ� ���������� �����ϰ� ������� �Լ��� ����

//----------lock
int lock_init(lock_data* lock) {
    return pthread_rwlock_init(lock, NULL);
}

int lock_destroy(lock_data* lock) {
    return pthread_rwlock_destroy(lock);
}

int lock_read_lock(lock_data* lock) {
    return pthread_rwlock_rdlock(lock);
}

int lock_read_unlock(lock_data* lock) {
    return pthread_rwlock_unlock(lock);
}

int lock_write_lock(lock_data* lock) {
    return pthread_rwlock_wrlock(lock);
}

int lock_write_unlock(lock_data* lock) {
    return pthread_rwlock_unlock(lock);
}
//----------lock

long long get_current_time_ms() {
    struct timespec spec;
    // CLOCK_MONOTONIC�� �ý��� �ð� ���濡 ������ ���� �ʴ� ���� ���� �ð��Դϴ�.<-??
    clock_gettime(CLOCK_MONOTONIC, &spec);
    long long ms = (long long)spec.tv_sec * 1000 + (long long)spec.tv_nsec / 1000000;
    return ms;
}
#endif