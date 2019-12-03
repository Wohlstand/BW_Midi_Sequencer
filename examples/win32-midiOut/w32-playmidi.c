#include <windows.h>
#include "native_midi_win32_seq.h"

int main(int argc, char**argv)
{
    double t = 0.001;
    void *q = mixer_seq_init_interface();
    mixer_seq_init_midi_out(q);

    mixer_seq_set_loop_enabled(q, 1);
    mixer_seq_openFile(q, argv[1]);

    while(!mixer_seq_at_end(q))
    {
        t = mixer_seq_tick(q, t, 0.001);
        Sleep((int)(t * 1000.0));
    }

    mixer_seq_close_midi_out(q);
    mixer_seq_free(q);

    return 0;
}
