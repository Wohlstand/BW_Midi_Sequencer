#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>

// Rename class to avoid ABI collisions
#define BW_MidiSequencer MixerMidiSequencer
// Inlucde MIDI sequencer class implementation
#include "midi_sequencer_impl.hpp"

#include "native_midi_win32_seq.h"

class MixerSeqInternal
{
public:
    MixerSeqInternal() :
        seq(),
        seq_if(NULL),
        out(NULL)
    {}
    ~MixerSeqInternal()
    {
        if(seq_if)
            delete seq_if;
    }
    MixerMidiSequencer seq;
    BW_MidiRtInterface *seq_if;
    HMIDIOUT out;
};

/****************************************************
 *           Real-Time MIDI calls proxies           *
 ****************************************************/

static inline unsigned int makeEvt(uint8_t t, uint8_t channel)
{
    return ((t << 4) & 0xF0) | (channel & 0x0F);
}

static void rtNoteOn(void *userdata, uint8_t channel, uint8_t note, uint8_t velocity)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(userdata);
    DWORD msg = 0;
    msg |= (velocity << 16) & 0xFF0000;
    msg |= (note << 8) & 0x00FF00;
    msg |= makeEvt(0x09, channel);
    midiOutShortMsg(seqi->out, msg);
}

static void rtNoteOff(void *userdata, uint8_t channel, uint8_t note)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(userdata);
    DWORD msg = 0;
    msg |= (note << 8) & 0x00FF00;
    msg |= makeEvt(0x08, channel);
    midiOutShortMsg(seqi->out, msg);
}

static void rtNoteAfterTouch(void *userdata, uint8_t channel, uint8_t note, uint8_t atVal)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(userdata);
    DWORD msg = 0;
    msg |= (atVal << 16) & 0xFF0000;
    msg |= (note << 8) & 0x00FF00;
    msg |= makeEvt(0x0A, channel);
    midiOutShortMsg(seqi->out, msg);
}

static void rtChannelAfterTouch(void *userdata, uint8_t channel, uint8_t atVal)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(userdata);
    DWORD msg = 0;
    msg |= (atVal << 8) & 0x00FF00;
    msg |= makeEvt(0x0D, channel);
    midiOutShortMsg(seqi->out, msg);
}

static void rtControllerChange(void *userdata, uint8_t channel, uint8_t type, uint8_t value)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(userdata);
    DWORD msg = 0;
    msg |= (value << 16) & 0xFF0000;
    msg |= (type << 8) & 0x00FF00;
    msg |= makeEvt(0x0B, channel);
    midiOutShortMsg(seqi->out, msg);
}

static void rtPatchChange(void *userdata, uint8_t channel, uint8_t patch)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(userdata);
    DWORD msg = 0;
    msg |= (patch << 8) & 0x00FF00;
    msg |= makeEvt(0x0C, channel);
    midiOutShortMsg(seqi->out, msg);
}

static void rtPitchBend(void *userdata, uint8_t channel, uint8_t msb, uint8_t lsb)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(userdata);
    DWORD msg = 0;
    msg |= (static_cast<int>(lsb) << 7) << 8;
    msg |= (static_cast<int>(msb) & 127) << 16;
    msg |= makeEvt(0x0E, channel);
    midiOutShortMsg(seqi->out, msg);
}

static void rtSysEx(void *userdata, const uint8_t *msg, size_t size)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(userdata);
    MIDIHDR hd;
    hd.dwBufferLength = size;
    hd.dwBytesRecorded = size;
    hd.lpData = (LPSTR)msg;/*(LPSTR)malloc(size);
    memcpy(hd.lpData, msg, size);*/
    midiOutPrepareHeader(seqi->out, &hd, sizeof(hd));
    midiOutLongMsg(seqi->out, &hd, sizeof(hd));
}


/* NonStandard calls */
//static void rtDeviceSwitch(void *userdata, size_t track, const char *data, size_t length)
//{

//}

//static size_t rtCurrentDevice(void *userdata, size_t track)
//{
//    return 0;
//}
/* NonStandard calls End */


int mixer_seq_init_midi_out(void *seq)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(seq);
    MMRESULT err = midiOutOpen(&seqi->out, 0, 0, 0, CALLBACK_NULL);
    if (err != MMSYSERR_NOERROR)
    {
       // ("error opening default MIDI device: %d\n", err);
       return -1;
    }
    return 0;
}

void mixer_seq_close_midi_out(void *seq)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(seq);
    if(seqi->out)
        midiOutClose(seqi->out);
    seqi->out = NULL;
}


void *mixer_seq_init_interface()
{
    MixerSeqInternal *seqi = new MixerSeqInternal;
    seqi->seq_if = new BW_MidiRtInterface;
    std::memset(seqi->seq_if, 0, sizeof(BW_MidiRtInterface));

    // seq->onDebugMessage             = hooks.onDebugMessage;
    // seq->onDebugMessage_userData    = hooks.onDebugMessage_userData;

    /* MIDI Real-Time calls */
    seqi->seq_if->rtUserData = seqi;
    seqi->seq_if->rt_noteOn  = rtNoteOn;
    seqi->seq_if->rt_noteOff = rtNoteOff;
    seqi->seq_if->rt_noteAfterTouch = rtNoteAfterTouch;
    seqi->seq_if->rt_channelAfterTouch = rtChannelAfterTouch;
    seqi->seq_if->rt_controllerChange = rtControllerChange;
    seqi->seq_if->rt_patchChange = rtPatchChange;
    seqi->seq_if->rt_pitchBend = rtPitchBend;
    seqi->seq_if->rt_systemExclusive = rtSysEx;

    /* NonStandard calls */
    //seqi->seq_if->rt_deviceSwitch = rtDeviceSwitch;
    //seqi->seq_if->rt_currentDevice = rtCurrentDevice;
    /* NonStandard calls End */

    seqi->seq.setInterface(seqi->seq_if);

    return seqi;
}

void mixer_seq_free(void *seq)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(seq);
    delete seqi->seq_if;
}

int mixer_seq_openData(void *seq, void *bytes, unsigned long len)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(seq);
    bool ret = seqi->seq.loadMIDI(bytes, static_cast<size_t>(len));
    return ret ? 0 : -1;
}

int mixer_seq_openFile(void *seq, const char *path)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(seq);
    bool ret = seqi->seq.loadMIDI(path);
    return ret ? 0 : -1;
}


const char *mixer_seq_meta_title(void *seq)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(seq);
    return seqi->seq.getMusicTitle().c_str();
}

const char *mixer_seq_meta_copyright(void *seq)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(seq);
    return seqi->seq.getMusicCopyright().c_str();
}

void mixer_seq_rewind(void *seq)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(seq);
    seqi->seq.rewind();
}


void mixer_seq_seek(void *seq, double pos)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(seq);
    seqi->seq.seek(pos, 1);
}

double mixer_seq_tell(void *seq)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(seq);
    return seqi->seq.tell();
}

double mixer_seq_length(void *seq)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(seq);
    return seqi->seq.timeLength();
}

double mixer_seq_loop_start(void *seq)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(seq);
    return seqi->seq.getLoopStart();
}

double mixer_seq_loop_end(void *seq)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(seq);
    return seqi->seq.getLoopEnd();
}

int mixer_seq_at_end(void *seq)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(seq);
    return seqi->seq.positionAtEnd() ? 1 : 0;
}


double mixer_seq_get_tempo_multiplier(void *seq)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(seq);
    return seqi->seq.getTempoMultiplier();
}

void mixer_seq_set_tempo_multiplier(void *seq, double tempo)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(seq);
    seqi->seq.setTempo(tempo);
}

void mixer_seq_set_loop_enabled(void *seq, int loopEn)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(seq);
    seqi->seq.setLoopEnabled(loopEn > 0);
}

double mixer_seq_tick(void *seq, double s, double granularity)
{
    MixerSeqInternal *seqi = reinterpret_cast<MixerSeqInternal*>(seq);
    double ret = seqi->seq.Tick(s, granularity);

    s *= seqi->seq.getTempoMultiplier();
    return ret;
}
