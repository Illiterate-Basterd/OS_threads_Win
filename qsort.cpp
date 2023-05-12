#include <Windows.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#define SWAP(a, b) int temp = a; a = b; b = temp;
#define MAX(a, b) a > b ? a : b
#define MIN(a, b) a < b ? a : b

using namespace std;

void quicksort(int *Arr, int first, int last);
DWORD WINAPI thread_entry(void* param);

HANDLE hMutex;
int* Array;
int TCNT, NUM_ELEM;

int main(void)
{
    fstream input("input.txt", ios::in);
    if(!input.is_open())
    {
        cerr << "Couldn't open the input.txt! Error: " << GetLastError() << endl;
        return -1;
    }

    fstream output("output.txt", ios::out);
    if(!output.is_open())
    {
        cerr << "Couldn't open the output.txt! Error: " << GetLastError() << endl;
        return -1;
    }

    fstream time_thread("time.txt", ios::out);
    if(!time_thread.is_open())
    {
        cerr << "Couldn't open the time.txt! Error: " << GetLastError() << endl;
        return -1;
    }

	size_t beg, end;
	input >> TCNT >> NUM_ELEM;
	Array = new int[NUM_ELEM];
	HANDLE* threads = new HANDLE[TCNT];

	for (size_t i = 0; i < NUM_ELEM; i++)
	{
		input >> Array[i];
	}

	hMutex = CreateMutex(NULL, FALSE, NULL);

	beg = GetTickCount64();
	for(int i = 0; i < TCNT; i++)
	{
		threads[i] = CreateThread(0, 0, thread_entry, (void*)((char*)0 + i), 0, 0);
	}
	end = GetTickCount64();

	CloseHandle(hMutex);
	for(int i = 0; i < TCNT; i++)
	{
		CloseHandle(threads[i]);
	}
	quicksort(Array, 0, NUM_ELEM - 1);
	

	output << TCNT << endl << NUM_ELEM << endl;
	
	for (size_t i = 0; i < NUM_ELEM; i++)
	{
		output << Array[i] << " ";
	}

	time_thread << end - beg << endl;

    return 0;
}

void quicksort(int *Arr, int first, int last)
{
	int i, j, pivot;
	int temp;
	if (first < last)
	{
		pivot = first;
		i = first;
		j = last;

		while (i < j)
		{
			while (Arr[i] <= Arr[pivot] && i < last)
				i++;

			while (Arr[j] > Arr[pivot])
				j--;

			if (i < j)
			{
				SWAP(Arr[i], Arr[j])
			}
		}
		SWAP(Arr[pivot], Arr[j]);
		quicksort(Arr, first, j - 1);
		quicksort(Arr, j + 1, last);
	}
}

DWORD WINAPI thread_entry(void* param)
{
	if(TCNT == 1 || NUM_ELEM <= 1000)
	{
		quicksort(Array, 0, NUM_ELEM - 1);
	}
	
	else
	{
		int idx = (char*)param - (char*)0;
		int sub1 = TCNT - idx;
		int sub2 = NUM_ELEM % TCNT;

		int arg1 = NUM_ELEM / sub1, arg2 = NUM_ELEM / sub2;

		WaitForSingleObject(hMutex, INFINITE);
		quicksort(Array, MIN(arg1, arg2), MAX(arg1, arg2));
		ReleaseMutex(hMutex);
	}
	return 0;
}