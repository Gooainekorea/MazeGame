#include "platform_abstraction.h"

#ifdef _WIN32 // Windows용 함수 tlqkf
void terminal_initial() {} //window에서는 딱히 터미널 초기화 필요 없다함
void terminal_exit() {} //종료도 딱히 필요없다함
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
    cursorInfo.bVisible = FALSE; // 커서의 bVisible 멤버를 FALSE로 설정
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void show_cursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = TRUE; // 커서의 bVisible 멤버를 TRUE로 설정
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}
//--------thread
int thread_create(thread_data* thread, thread_type start_routine, void* g_structs) {
    // Windows API 사용
    *thread = CreateThread(
        NULL,                   // 기본 보안 속성
        0,                      // 기본 스택 크기
        (LPTHREAD_START_ROUTINE)start_routine, // 스레드 함수
        g_structs,                    // 함수에 전달할 인자
        0,                      // 즉시 실행
        NULL                    // 스레드 ID저장 위치- NULL : 저장 안함
    );
    if (*thread == NULL) {
        return -1; // 실패
    }
    return 0; // 성공
}
int thread_join(thread_data thread) {
    // Windows API 사용
    DWORD result = WaitForSingleObject(thread, INFINITE); //스레드 종료까지 대기 INFINITE : 무한 대기
    CloseHandle(thread); // 핸들 닫기(리소스 해제)
    return (result == WAIT_OBJECT_0) ? 0 : -1;// 0: 성공, 1: 실패
}
//--------thread
//cls...
void clear_screen() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordScreen = { 0, 0 };    // 홈 커서 위치
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    // 콘솔 화면 버퍼 정보를 가져옵니다.
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return;
    }

    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    // 콘솔 화면을 공백으로 채웁니다.
    FillConsoleOutputCharacter(hConsole, (TCHAR)' ', dwConSize, coordScreen, &cCharsWritten);

    // 콘솔 화면의 속성(색상 등)을 현재 속성으로 다시 채웁니다.
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten);

    // 커서를 (0, 0)으로 이동시킵니다.
    SetConsoleCursorPosition(hConsole, coordScreen);
}//뭐야 이거 왤케 복잡해?? cls쓸줄 알았더니? cls 보안위협및 ncurses와 충돌위험있어서 API호출해서 화면제어했다함

//----------lock
int lock_init(lock_data* lock) {//잠금해제
    InitializeSRWLock(lock);
    return 0;
}

int lock_destroy(lock_data* lock) {//잠금 해
    (void)lock; // SRWLock은 명시적인 파괴 함수가 필요 없음
    return 0;
}

int lock_read_lock(lock_data* lock) {//읽기 잠금
    AcquireSRWLockShared(lock);
    return 0;
}

int lock_read_unlock(lock_data* lock) {//읽기 잠금 해제
    ReleaseSRWLockShared(lock);
    return 0;
}

int lock_write_lock(lock_data* lock) {//쓰기 잠금
    AcquireSRWLockExclusive(lock);
    return 0;
}

int lock_write_unlock(lock_data* lock) {//쓰기 잠금 해제
    ReleaseSRWLockExclusive(lock);
    return 0;
}

//----------lock

long long get_current_time_ms() {
    return GetTickCount64();
}

#else // linux용 함수 Tlqkf

void terminal_initial() { //터미널 초기화
    initscr();             // ncurses 모드 시작
    cbreak();              // Line buffering 비활성화
    noecho();              // 입력된 키 숨김
    keypad(stdscr, TRUE);  // 특수 키(F1, 화살표 등) 사용 가능
    curs_set(0);           // 커서 숨김
    nodelay(stdscr, TRUE); // getch()를 non-blocking으로 만듦
    mysleep(10);
}
void terminal_exit() { //터미널 종료
    endwin(); // ncurses 모드 종료
}
void gotoxy(int x, int y) {
    move(y, x); // ncurses 함수
}
void mysleep(int milliseconds) {
    usleep(milliseconds * 1000); // usleep은 마이크로초 단위
}
int mykbhit() {
    int ch = getch();
    if (ch != ERR) {
        ungetch(ch); // 읽은 문자를 다시 버퍼에 넣음
        return 1;
    }
    else {
        return 0;
    }
}
int mygetch() {
    return getch(); // nodelay가 설정되어 있어 non-blocking으로 동작
}
void console_size(int* rows, int* cols) {
    getmaxyx(stdscr, *rows, *cols);
}
void hide_cursor() {
    curs_set(0); // ncurses에서 커서를 보이지 않게 설정
}

void show_cursor() {
    curs_set(1); // ncurses에서 커서를 보이게 설정
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
    clear();    // ncurses: 화면 버퍼를 지웁니다.
    refresh();  // ncurses: 실제 터미널에 변경 사항을 적용합니다.
}//윈도우를 희생하고 리눅스에서 간편하게 제어가능한 함수를 얻음

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
    // CLOCK_MONOTONIC은 시스템 시간 변경에 영향을 받지 않는 단조 증가 시간입니다.<-??
    clock_gettime(CLOCK_MONOTONIC, &spec);
    long long ms = (long long)spec.tv_sec * 1000 + (long long)spec.tv_nsec / 1000000;
    return ms;
}
#endif