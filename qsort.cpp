#include <Windows.h>
#include <iostream>
#include <fstream>
#define SWAP(a, b) int temp = a; a = b; b = temp;

using namespace std;

void quicksort(int *Arr, int first, int last);
DWORD WINAPI thread_entry(void* param);

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

	int TCNT, NUM_ELEM;
	input >> TCNT >> NUM_ELEM;
	int* Array = new int[NUM_ELEM];

	for (size_t i = 0; i < NUM_ELEM; i++)
	{
		input >> Array[i];
	}

	quicksort(Array, 0, NUM_ELEM - 1);

	output << TCNT << endl << NUM_ELEM << endl;
	
	for (size_t i = 0; i < NUM_ELEM; i++)
	{
		output << Array[i] << " ";
	}

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