#define PHIL_COUNT 5
#include <Windows.h>
#include <stdlib.h>
#include <iostream>

typedef struct ph
{
    short id;
    ph *phil_left;
    ph *phil_right;
    HANDLE phil_event;
} PHIL;

using namespace std;

PHIL phils[PHIL_COUNT];
HANDLE threads[PHIL_COUNT];
HANDLE sema, sema_1, event;
DWORD starting_time;
int time_total, time_phil, eaten;
double elapsed = 0;
LARGE_INTEGER beg, fin, freq;

void init(void);
DWORD WINAPI thread_entry(void *param);
bool eating(PHIL *ph_l, PHIL *ph_r, int ph_id);

int main(int argc, char* argv[])
{
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&beg);

    QueryPerformanceCounter(&fin);
    elapsed += (double)(fin.QuadPart - beg.QuadPart) / freq.QuadPart * 1000;
    starting_time = GetTickCount();
    if (argc != 3)
    {
        cerr << "Wrong amount of args!" << endl;
        return -1;
    }

    time_total = atoi(argv[1]);
    time_phil = atoi(argv[2]);
    init();

    event = CreateEvent(NULL, TRUE, FALSE, NULL);
    sema = CreateSemaphore(0, 1, 1, 0);
    sema_1 = CreateSemaphore(0, 1, 1, 0);
    for(int i = 0; i < PHIL_COUNT; i++)
        phils[i].phil_event = CreateEvent(NULL, TRUE, TRUE, NULL);
    

    for(int i = 0; i < PHIL_COUNT; i++)
    {
        threads[i] = CreateThread(0, 0, thread_entry, (void *)((char *)0 + i), 0, 0);
    }

    WaitForMultipleObjects(PHIL_COUNT, threads, TRUE, INFINITE);

    CloseHandle(event);
    CloseHandle(sema_1);
    CloseHandle(sema);
    for(int i = 0; i < PHIL_COUNT; i++)
    {
        CloseHandle(phils[i].phil_event);
        CloseHandle(threads[i]);
    }

    return 0;
}

void init(void)
{
    for(int i = 0; i < PHIL_COUNT; i++)
    {
        if (i == 0)
        {
            phils[i].id = 1;
            phils[i].phil_left = &phils[PHIL_COUNT - 1];
            phils[i].phil_right = &phils[1];
        }

        else if (i == 4)
        {
            phils[i].id = PHIL_COUNT;
            phils[i].phil_left = &phils[i - 1];
            phils[i].phil_right = &phils[0];
        }

        else
        {
        phils[i].id = i + 1;
        phils[i].phil_left = &phils[i - 1];
        phils[i].phil_right = &phils[i + 1];
        }
    }
    eaten = 0;
}

DWORD WINAPI thread_entry(void *param)
{
    int idx = (char *)param - (char *)0;
    QueryPerformanceCounter(&fin);
    elapsed += (double)(fin.QuadPart - beg.QuadPart) / freq.QuadPart * 1000;

    while (GetTickCount() - starting_time < time_total)
    {

        if(eating(phils[idx].phil_left, phils[idx].phil_right, phils[idx].id))
            break;
    }

    if (++eaten < PHIL_COUNT - 1)
    {
        WaitForSingleObject(event, INFINITE);
        return 0;
    }
    else
    {
        SetEvent(event);
        return 0;
    }
}

void print_info(DWORD start_eat_time, int ph_id)
{
    WaitForSingleObject(sema, INFINITE);
    QueryPerformanceCounter(&fin);
    elapsed += (double)(fin.QuadPart - beg.QuadPart) / freq.QuadPart * 1000;
    cout << start_eat_time - starting_time << ":" <<  ph_id << ":T->E" << endl;
    ReleaseSemaphore(sema, 1, NULL);
    Sleep(time_phil);
    WaitForSingleObject(sema, INFINITE);

    cout << time_phil + start_eat_time - starting_time << ":" <<  ph_id << ":E->T" << endl;
    ReleaseSemaphore(sema, 1, NULL);
}

bool eating(PHIL *ph_l, PHIL *ph_r, int ph_id)
{
    DWORD start_eat_time;
    volatile HANDLE* evs = new HANDLE[2];
    WaitForSingleObject(sema_1, INFINITE);
    evs[0] = ph_l->phil_event;
    evs[1] = ph_r->phil_event;
    ReleaseSemaphore(sema_1, 1, NULL);

    WaitForMultipleObjects(2, (HANDLE *)evs, true, INFINITE);
    WaitForSingleObject(sema_1, INFINITE);
    ResetEvent(phils[ph_id - 1].phil_event);
    ReleaseSemaphore(sema_1, 1, NULL);

    QueryPerformanceCounter(&fin);
    elapsed += (double)(fin.QuadPart - beg.QuadPart) / freq.QuadPart * 1000;
    start_eat_time = GetTickCount();

    if (GetTickCount() - starting_time > time_total)
    {
        SetEvent(phils[ph_id - 1].phil_event);
        delete[] evs;
        return true;
    }

    else if (GetTickCount() - starting_time < time_total)
    print_info(start_eat_time, ph_id);

    SetEvent(phils[ph_id - 1].phil_event);
    Sleep(time_phil - time_phil / 100);
    delete[] evs;
    return false;
}