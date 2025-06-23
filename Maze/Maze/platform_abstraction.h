#pragma once
// OS�� include ���
#ifdef _WIN32 //windows
#include <Windows.h> //win api
#include <conio.h>	//�ܼ� ����� �Լ�
typedef HANDLE thread_data; //������ ������ : ������ �ĺ��� ������ ���� �ü���� �̸� �����س��� ����
typedef SRWLOCK lock_data; //win�� lock Ÿ��

#else // tlqkf
#include <ncurses.h>	//�ý�Ʈ��� ui���̺귯��
#include <unistd.h>		//ǥ���Լ� ���
#include <pthread.h>	//������ ���� �Լ�
typedef pthread_t thread_data;//������ ������ Ÿ�Ը� ����
typedef pthread_rwlock_t lock_data; // Linux�� lock

#endif //�ڽŰ� ¦�� �´� #ifdef, #ifndef, #if�� ���� �ǹ�
void terminal_initial(); //�͹̳� �ʱ�ȭ
void terminal_exit(); //�͹̳� ����
void gotoxy(int x, int y);
void mysleep(int milliseconds);//���� sleep �Լ� �̹� �������� �빮�ڶ��
int mykbhit();
int mygetch();//���� getch�� ���������� �ֳ�
void console_size(int* rows, int* cols);//���⿡ int�ּҸ� ������ ���� �ܼ� ������� ���ϰ� �ٲ��� ������ �����ʿ��� ������
void show_cursor();
void hide_cursor();
void clear_screen();

// Tlqkf���� �ð��� �и���(ms) ������ ��ȯ
long long get_current_time_ms();//����ð�

typedef void* (*thread_type)(void*); // ������ �Լ������� Ÿ�� ����
// ������ ����
int thread_create(thread_data* thread, thread_type start_routine, void* g_structs);
// thread_data* thread : ������ ������ �ĺ��ڸ� ������ ������, 
// thread_type start_routine : �����忡�� ����� �Լ�, 
// void* arg : �Լ��� ������ ����

// ������ ���
int thread_join(thread_data thread);
// thread_data thread : ��� ������ �ĺ���


int lock_init(lock_data* lock);		// �� �ʱ�ȭ
int lock_destroy(lock_data* lock);	// �� �ı�
int lock_read_lock(lock_data* lock);	// �б�� (���� ��) ȹ�� 
int lock_read_unlock(lock_data* lock);	// �б� �� ����
int lock_write_lock(lock_data* lock);	// ���� ��(��Ÿ ��) ȹ��
int lock_write_unlock(lock_data* lock);	// ���� �� ����