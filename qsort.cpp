#include <Windows.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#define SWAP(a, b) \
	int temp = a;  \
	a = b;         \
	b = temp;
#define MAX(a, b) a > b ? a : b
#define MIN(a, b) a < b ? a : b

using namespace std;

void quicksort(int *Arr, int first, int last);
DWORD WINAPI thread_entry(void *param);
bool check_sort(int *Arr);

HANDLE hMutex;
int *Array;
HANDLE *threads;
int TCNT, NUM_ELEM, TCNT_C;
LARGE_INTEGER beg, fin, freq;
double elapsed = 0.f;

int main(void)
{
	fstream input("input.txt", ios::in);
	if (!input.is_open())
	{
		cerr << "Couldn't open the input.txt! Error: " << GetLastError() << endl;
		return -1;
	}

	fstream output("output.txt", ios::out);
	if (!output.is_open())
	{
		cerr << "Couldn't open the output.txt! Error: " << GetLastError() << endl;
		return -1;
	}

	fstream time_thread("time.txt", ios::out);
	if (!time_thread.is_open())
	{
		cerr << "Couldn't open the time.txt! Error: " << GetLastError() << endl;
		return -1;
	}

	input >> TCNT >> NUM_ELEM;
	Array = new int[NUM_ELEM];
	threads = new HANDLE[TCNT];
	TCNT_C = NUM_ELEM / TCNT;

	for (size_t i = 0; i < NUM_ELEM; i++)
	{
		input >> Array[i];
	}

	QueryPerformanceFrequency(&freq);

	if (TCNT == 1 || NUM_ELEM <= 1000)
	{
		QueryPerformanceCounter(&beg);
		quicksort(Array, 0, NUM_ELEM - 1);
		QueryPerformanceCounter(&fin);
		elapsed += (double)(fin.QuadPart - beg.QuadPart) / freq.QuadPart * 1000;
	}

	else
	{

		hMutex = CreateMutex(NULL, FALSE, NULL);

		for (int i = 0; i < TCNT; i++)
		{
			threads[i] = CreateThread(0, 0, thread_entry, (void *)((char *)0 + i), 0, 0);
		}

		while(true)
		{
			if(WAIT_TIMEOUT == WaitForMultipleObjects(TCNT, threads, TRUE, 50))
				continue;
			else
			{
				QueryPerformanceCounter(&beg);
				quicksort(Array, 0, NUM_ELEM - 1);
				QueryPerformanceCounter(&fin);
				elapsed += (double)(fin.QuadPart - beg.QuadPart) / freq.QuadPart * 1000;
				break;
			}
		}

		CloseHandle(hMutex);
		for (int i = 0; i < TCNT; i++)
		{
			CloseHandle(threads[i]);
		}
		delete[] threads;
	}

	output << TCNT << endl
		   << NUM_ELEM << endl;

	for (size_t i = 0; i < NUM_ELEM; i++)
	{
		output << Array[i] << " ";
	}

	time_thread << (int)elapsed << endl;

	//check_sort(Array);

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

DWORD WINAPI thread_entry(void *param)
{
	WaitForSingleObject(hMutex, INFINITE);

	int idx = (char *)param - (char *)0;
	// cout << idx << endl;
	volatile int arg1 = TCNT_C * idx;
	volatile int arg2 = arg1 + TCNT_C - 1;
	//cout << arg1 << " " << arg2 << endl;

	QueryPerformanceCounter(&beg);
	quicksort(Array, arg1, arg2);
	QueryPerformanceCounter(&fin);
	elapsed += (double)(fin.QuadPart - beg.QuadPart) / freq.QuadPart * 1000;

	ReleaseMutex(hMutex);
	return 0;
}

bool check_sort(int *Arr)
{
	int i;
	for (i = 1; i < NUM_ELEM; i++)
	{
		if (Arr[i - 1] > Arr[i])
		{
			cout << "Wrong sort!" << endl;
			cout << Arr[i - 1] << " " << Arr[i] << endl;
			return false;
		}
	}

	if (i != NUM_ELEM)
	{
		cout << "Not enough elements!" << endl;
		return false;
	}

	cout << "Sorted" << endl;
	return true;
}