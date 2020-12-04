#include <windows.h>
#include <stdio.h>
#include <time.h>
#include "native_midi_win32_seq.h"

int main(int argc, char **argv)
{
    HANDLE hTimer = NULL;
    LARGE_INTEGER liDueTime;
    double t = 0.001, w;
    void *q;
    clock_t start, end;

    if(argc < 2)
    {
        fprintf(stderr, "\nUsage: %s <path\\to\\MIDI\\file.mid>\n\n", argv[0]);
        fflush(stderr);
        return 1;
    }

    liDueTime.QuadPart = -10000000LL;

    // Create an unnamed waitable timer.
    hTimer = CreateWaitableTimerW(NULL, TRUE, NULL);
    if (NULL == hTimer)
    {
        fprintf(stderr, "CreateWaitableTimer failed (%lu)\n", (unsigned long)GetLastError());
        fflush(stderr);
        return 1;
    }

    q = mixer_seq_init_interface();
    mixer_seq_init_midi_out(q);

    mixer_seq_set_loop_enabled(q, 1);
    if(mixer_seq_openFile(q, argv[1]) < 0)
    {
        fprintf(stderr, "Can't open file %s\n\n", argv[1]);
        fflush(stderr);
        return 0;
    }

    while(!mixer_seq_at_end(q))
    {
        start = clock();
        t = mixer_seq_tick(q, t, 0.0001);
        end = clock();

        w = (double)(end - start) / CLOCKS_PER_SEC;

        if((t - w) > 0.0)
        {
            liDueTime.QuadPart = (LONGLONG)-((t - w) * 10000000.0);
            if(!SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0))
            {
                printf("SetWaitableTimer failed (%lu)\n", (unsigned long)GetLastError());
                return 2;
            }

            if(WaitForSingleObject(hTimer, INFINITE) != WAIT_OBJECT_0)
                printf("WaitForSingleObject failed (%lu)\n", (unsigned long)GetLastError());
        }
    }

    mixer_seq_close_midi_out(q);
    mixer_seq_free(q);

    CloseHandle(hTimer);

    return 0;
}
