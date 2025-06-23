#pragma once
// OS별 include 헤더
#ifdef _WIN32 //windows
#include <Windows.h> //win api
#include <conio.h>	//콘솔 입출력 함수
typedef HANDLE thread_data; //스레드 데이터 : 스레드 식별과 관리를 위해 운영체제가 미리 정ㅎ해놓은 구조
typedef SRWLOCK lock_data; //win용 lock 타입

#else // tlqkf
#include <ncurses.h>	//택스트기반 ui라이브러리
#include <unistd.h>		//표준함수 사용
#include <pthread.h>	//스레드 관련 함수
typedef pthread_t thread_data;//스레드 데이터 타입명 통일
typedef pthread_rwlock_t lock_data; // Linux용 lock

#endif //자신과 짝이 맞는 #ifdef, #ifndef, #if의 끝을 의미
void terminal_initial(); //터미널 초기화
void terminal_exit(); //터미널 종료
void gotoxy(int x, int y);
void mysleep(int milliseconds);//ㅇㄴ sleep 함수 이미 존재했음 대문자라며
int mykbhit();
int mygetch();//뭐야 getch가 리눅스에도 있네
void console_size(int* rows, int* cols);//여기에 int주소를 넣으면 값을 콘솔 사이즈로 뿅하고 바꿔줌 포인터 리턴필요없어서 개편함
void show_cursor();
void hide_cursor();
void clear_screen();

// Tlqkf현재 시간을 밀리초(ms) 단위로 반환
long long get_current_time_ms();//현재시간

typedef void* (*thread_type)(void*); // 스레드 함수포인터 타입 통일
// 스레드 생성
int thread_create(thread_data* thread, thread_type start_routine, void* g_structs);
// thread_data* thread : 생성된 스레드 식별자를 저장할 포인터, 
// thread_type start_routine : 스레드에서 실행될 함수, 
// void* arg : 함수에 전달할 인자

// 스레드 대기
int thread_join(thread_data thread);
// thread_data thread : 대기 스레드 식별자


int lock_init(lock_data* lock);		// 락 초기화
int lock_destroy(lock_data* lock);	// 락 파괴
int lock_read_lock(lock_data* lock);	// 읽기락 (공유 락) 획득 
int lock_read_unlock(lock_data* lock);	// 읽기 락 해제
int lock_write_lock(lock_data* lock);	// 쓰기 락(배타 락) 획득
int lock_write_unlock(lock_data* lock);	// 쓰기 락 해제