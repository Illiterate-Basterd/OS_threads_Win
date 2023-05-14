#include <iostream>
#include <fstream>
#include <queue>
#include <Windows.h>
#define PHILCNT 5
#define TCNT 5

using namespace std;

enum state
{
    Thinking,
    Eating
};
LARGE_INTEGER beg, fin, freq;
CRITICAL_SECTION cs;
volatile double elapsed = 0;
int TOTAL, PHIL;
HANDLE sema;

struct philos
{
    unsigned short num;
    state State;
    unsigned short adjacent[2];
};

philos Philosophers[PHILCNT];
queue<philos> phils;

DWORD WINAPI Phil_threads(void *param)
{
    int idx = (char *)param - (char *)0;
    phils.push(Philosophers[idx]);
    philos Current_phil;

    while (elapsed <= TOTAL)
    {
        WaitForSingleObject(sema, INFINITE);
        QueryPerformanceCounter(&beg);

        EnterCriticalSection(&cs);
        Current_phil = phils.front();

        if (Philosophers[Current_phil.adjacent[0] - 1].State != Eating && Philosophers[Current_phil.adjacent[1]].State != Eating)
        {
            QueryPerformanceCounter(&fin);
            elapsed += (double)(fin.QuadPart - beg.QuadPart) / freq.QuadPart * 1000;

            if (elapsed <= TOTAL)
            {
                QueryPerformanceCounter(&beg);

                Current_phil.State = Eating;
                phils.pop();

                QueryPerformanceCounter(&fin);
                elapsed += (double)(fin.QuadPart - beg.QuadPart) / freq.QuadPart * 1000;
                cout << (int)elapsed << ":" << idx + 1 << ":T->E" << endl;

                QueryPerformanceCounter(&beg);
                LeaveCriticalSection(&cs);

                Sleep(PHIL);

                EnterCriticalSection(&cs);
                Current_phil.State = Thinking;
                phils.push(Current_phil);
                QueryPerformanceCounter(&fin);

                elapsed += (double)(fin.QuadPart - beg.QuadPart) / freq.QuadPart * 1000;
                cout << (int)elapsed << ":" << idx + 1 << ":E->T" << endl;
                LeaveCriticalSection(&cs);
            }

            else
                LeaveCriticalSection(&cs);
        }

        else
        {
            LeaveCriticalSection(&cs);
        }

        ReleaseSemaphore(sema, 1, NULL);
    }

    return 0;
}

int main(int argc, char* argv[])
{
    TOTAL = atoi(argv[1]);
    PHIL = atoi(argv[2]);

    for (unsigned short i = 0; i < PHILCNT; i++)
    {
        unsigned short t = i + 1;
        Philosophers[i].num = t;
        Philosophers[i].State = Thinking;

        if (t == 1)
        {
            Philosophers[i].adjacent[0] = t + 1;
            Philosophers[i].adjacent[1] = 5;
        }

        else if (t == 5)
        {
            Philosophers[i].adjacent[0] = t - 1;
            Philosophers[i].adjacent[1] = 1;
        }

        else
        {
            Philosophers[i].adjacent[0] = t - 1;
            Philosophers[i].adjacent[1] = t + 1;
        }
    }

    HANDLE threads[TCNT];
    sema = CreateSemaphore(0, 2, 2, 0);
    TOTAL -= TOTAL / 100;
    InitializeCriticalSection(&cs);
    QueryPerformanceFrequency(&freq);

    for (unsigned short i = 0; i < TCNT; i++)
    {
        threads[i] = CreateThread(0, 0, Phil_threads, (void *)((char *)0 + i), 0, 0);
    }

    while (true)
    {
        if (WAIT_TIMEOUT == WaitForMultipleObjects(TCNT, threads, TRUE, 50))
            continue;
        else
        {
            break;
        }
    }

    DeleteCriticalSection(&cs);
    for (int i = 0; i < TCNT; i++)
    {
        CloseHandle(threads[i]);
    }

    return 0;
}