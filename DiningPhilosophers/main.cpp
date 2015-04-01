/*
	Cornelius Donley
	Andrew Bockus
*/

// Win 32 app for the Dining Philosophers
// Shell by S. Renk  2012

#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <stdio.h>
#include <time.h>

using namespace std;

// shared vars for the philos - you can add more
enum pState { GONE, PRESENT, THINKING, HUNGRY, EATING, TALKING };
int philoCount = 0;                    // # of active philosophers
int forks[5] = { 1, 1, 1, 1, 1 };             // forks on table  1=present 0=gone
int usedForkLast[5] = { -1, -1, -1, -1, -1 }; // no one has used them last
pState philoState[5] = { GONE };           // 1-present 2-thinking 3-eating
int eatCount[5] = { 0 };                  // holds the # of time a philo has eaten
clock_t startTime, endTime;                       // used for run time calculation

// locks to create mutual exclusion
HANDLE mutex = CreateMutex(NULL, FALSE, NULL);
HANDLE forkLock[5];                  // these are the forks now

// ********** Create 5 philosophers & set them to eating ****************
void main()
{ // Set up 5 threads
	HANDLE pThread[5];                  // 5 philosopher tbreads
	DWORD pThreadID[5];                 // PID of thread
	DWORD WINAPI philosopher(LPVOID);   // code for the 5 philos

	cout << "Dining Philosophers - Cornelius Donley and Andrew Bockus\n\n";
	startTime = clock();              // start the timer

	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	printf("Start time and date: %s\n", asctime(timeinfo));

	// start 5 philosopher threads
	for (int philoNbr = 0; philoNbr < 5; philoNbr++)
	{
		pThread[philoNbr] = CreateThread(NULL, 0, philosopher, NULL, 0, &pThreadID[philoNbr]);
		forkLock[philoNbr] = CreateMutex(NULL, FALSE, NULL);
	}

	WaitForMultipleObjects(5, pThread, true, INFINITE); // wait for philos to finish
	endTime = clock();              // end the timer

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	printf("\nEnd time and date: %s", asctime(timeinfo));

	cout << "\nRun time = " << ((endTime - startTime) * 1000) / CLOCKS_PER_SEC << "ms\n";
	cout << "press CR to end."; while (_getch() != '\r');
	return;
}

// *************** The Philosopher **************************
DWORD WINAPI philosopher(LPVOID)
{
	int me;								  // holds my philo #
	int left, right;                      // philo on left & right
	int leftFork = 0, rightFork = 0;      // remember to reassign these to the correct values

	// who am I?
	WaitForSingleObject(mutex, INFINITE); // lock the lock
	me = philoCount++;
	ReleaseMutex(mutex);                  // unlock the lock
	philoState[me] = PRESENT;             // I’m here!
	left = (me + 4) % 5; right = (me + 1) % 5;     // define neighbors

	// assign forks
	leftFork = (me + 4) % 5;
	rightFork = me;

	// wait for everybody to show up to dinner
	while (philoCount < 5) Sleep(0);      // preempt self till everybody gets here

	// Is the print out jumbled? - why? fix
	WaitForSingleObject(mutex, INFINITE);
	cout << "Philosopher # " << me << " ready to dine." << endl;
	ReleaseMutex(mutex);

	while (eatCount[me] < 100) // eat 100 times
	{
		// think for awhile
		philoState[me] = THINKING;
		Sleep((DWORD) rand() % 20);

		philoState[me] = HUNGRY;
		// if (you can get forks) eat for awhile
		// pick up forks
		if (me % 2 == 0)
		{
			while (usedForkLast[leftFork] == me && philoState[left] == HUNGRY) Sleep(0);
			WaitForSingleObject(forkLock[leftFork], INFINITE);
			forks[leftFork]--;
			while (usedForkLast[rightFork] == me && philoState[right] == HUNGRY) Sleep(0);
			WaitForSingleObject(forkLock[rightFork], INFINITE);
			forks[rightFork]--;
		}// end even
		else
		{
			while (usedForkLast[rightFork] == me && philoState[right] == HUNGRY) Sleep(0);
			WaitForSingleObject(forkLock[rightFork], INFINITE);
			forks[rightFork]--;
			while (usedForkLast[leftFork] == me && philoState[left] == HUNGRY) Sleep(0);
			WaitForSingleObject(forkLock[leftFork], INFINITE);
			forks[leftFork]--;
		}// end odd		

		// eat for a while
		philoState[me] = EATING;
		eatCount[me]++;

		// error check
		WaitForSingleObject(mutex, INFINITE);
		if (philoState[left] == EATING || philoState[right] == EATING)
			cout << "******** Eating Error ******** " << endl;
		ReleaseMutex(mutex);

		WaitForSingleObject(mutex, INFINITE);
		if (forks[leftFork] || forks[rightFork])
			cout << "******** Fork Error ******** "  << endl;
		ReleaseMutex(mutex);				

		Sleep((DWORD)rand() % 20);

		// return forks
		philoState[me] = TALKING;
		usedForkLast[leftFork] = me;
		forks[leftFork]++;
		ReleaseMutex(forkLock[leftFork]);
		usedForkLast[rightFork] = me;
		forks[rightFork]++;		
		ReleaseMutex(forkLock[rightFork]);

		/*
		// check to pake sure the philiosphers are eating
		WaitForSingleObject(mutex, INFINITE);
		if (eatCount[me] % 1000 == 0)
			cout << "Philiosopher #" << me << " has eaten " << eatCount[me] << " times." << endl;
		ReleaseMutex(mutex);
		*/

		// talk for a while		
		Sleep(rand() % 10);
	}

	WaitForSingleObject(mutex, INFINITE); // lock the lock	
	cout << "philosopher # " << me << " is leaving now." << endl;
	for (int i = 0; i<5; cout << eatCount[i++] << " "); cout << endl;
	ReleaseMutex(mutex);                  // unlock the lock
	philoState[me] = GONE;
	return 0;
}

/*
Dining Philosophers - Cornelius Donley and Andrew Bockus

Start time and date: Tue Feb 17 11:54:33 2015

Philosopher # 0 ready to dine.
Philosopher # 3 ready to dine.
Philosopher # 2 ready to dine.
Philosopher # 1 ready to dine.
Philosopher # 4 ready to dine.
philosopher # 0 is leaving now.
1000000 992029 986381 979709 970456
philosopher # 1 is leaving now.
1000000 1000000 994080 987360 978445
philosopher # 2 is leaving now.
1000000 1000000 1000000 993006 984246
philosopher # 3 is leaving now.
1000000 1000000 1000000 1000000 991264
philosopher # 4 is leaving now.
1000000 1000000 1000000 1000000 1000000

End time and date: Tue Feb 17 21:45:52 2015

Run time = 1120047ms
press CR to end.
*/