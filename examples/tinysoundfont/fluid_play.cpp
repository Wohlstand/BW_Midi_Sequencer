#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include "fluid_seq.h"
#include <signal.h>

enum { nch = 2 };
enum { buffer_size = 4096 };

/* variable declarations */
static Uint32 is_playing = 0; /* remaining length of the sample we have to play */

static void sig_playing(int)
{
    is_playing = 0;
}

/*
 audio callback function
 here you have to copy the data of your audio buffer into the
 requesting audio buffer (stream)
 you should only copy as much as the requested length (len)
*/
void my_audio_callback(void *midi_player, Uint8 *stream, int len)
{
    FluidMidiSeq *mp = (FluidMidiSeq*)midi_player;
    int got = mp->playBuffer(stream, len);
    if(got == 0)
        is_playing = 0;
}

int main(int argc, char **argv)
{
    static SDL_AudioSpec spec;
    FILE *test;

    if(argc<=2)
    {
        printf("%s filename.mid soundfont.sf2\n", argv[0]);
        return 0;
    }

    test = fopen(argv[2], "rb");
    if(!test)
    {
        printf("SoundFont file open error!\n");
        return 1;
    }
    fclose(test);


    FluidMidiSeq m(argv[2], 44100);

    if(!m.Open(argv[1]))
    {
        printf("MIDI file open error!\n");
        return 1;
    }

    m.setLoop(true);

    /* Initialize SDL.*/
    if(SDL_Init(SDL_INIT_AUDIO) < 0)
        return 1;

    signal(SIGINT, &sig_playing);
    signal(SIGTERM, &sig_playing);

    memset(&spec, 0, sizeof(SDL_AudioSpec));
    spec.freq = 44100;
    spec.format = AUDIO_S16SYS;
    spec.channels = 2;
    spec.samples = buffer_size;

    /* set the callback function */
    spec.callback = my_audio_callback;
    /* set ADLMIDI's descriptor as userdata to use it for sound generation */
    spec.userdata = &m;

    /* Open the audio device */
    if (SDL_OpenAudio(&spec, nullptr) < 0)
    {
        fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
        return 1;
    }

    is_playing = 1;
    /* Start playing */
    SDL_PauseAudio(0);

    printf("Playing... Hit Ctrl+C to quit!\n");

    /* wait until we're don't playing */
    while(is_playing)
    {
        SDL_Delay(100);
    }

    /* shut everything down */
    SDL_CloseAudio();

    printf("\n");
    fflush(stdout);

    return 0;
}
