#include <stdio.h>
// クラスの名前を変更してABIの衝突を回避する
#define BW_MidiSequencer FluidMidiSequencer
// MIDIシーケンサークラスの実装を含める
#include "midi_sequencer_impl.hpp"

#include "fluid_seq.h"

#define TSF_IMPLEMENTATION
#include "tsf/tsf.h"

/****************************************************
 *           リアルタイムMIDI呼び出しプロキシ            *
 ****************************************************/

FluidMidiSeq &getModule(void *userdata)
{
    FluidMidiSeq *c = reinterpret_cast<FluidMidiSeq *>(userdata);
    return *c;
}

static void rtNoteOn(void *userdata, uint8_t channel, uint8_t note, uint8_t velocity)
{
    FluidMidiSeq &m = getModule(userdata);
    tsf_channel_note_on(m.getSynth(), channel, note, static_cast<float>(velocity) / 127.0f);
}

static void rtNoteOff(void *userdata, uint8_t channel, uint8_t note)
{
    FluidMidiSeq &m = getModule(userdata);
    tsf_channel_note_off(m.getSynth(), channel, note);
}

static void rtNoteAfterTouch(void *userdata, uint8_t channel, uint8_t note, uint8_t atVal)
{
    (void)userdata; (void)channel; (void)note; (void)atVal;
    // サポートされていません
}

static void rtChannelAfterTouch(void *userdata, uint8_t channel, uint8_t atVal)
{
    (void)userdata; (void)channel; (void)atVal;
    // サポートされていません
}

static void rtControllerChange(void *userdata, uint8_t channel, uint8_t type, uint8_t value)
{
    FluidMidiSeq &m = getModule(userdata);
    tsf_channel_midi_control(m.getSynth(), channel, type, value);
}

static void rtPatchChange(void *userdata, uint8_t channel, uint8_t patch)
{
    FluidMidiSeq &m = getModule(userdata);
    tsf_channel_set_presetnumber(m.getSynth(), channel, patch, channel == 9);
}

static void rtPitchBend(void *userdata, uint8_t channel, uint8_t msb, uint8_t lsb)
{
    FluidMidiSeq &m = getModule(userdata);
    tsf_channel_set_pitchwheel(m.getSynth(), channel, (msb << 7) | lsb);
}

static void rtSysEx(void *userdata, const uint8_t *msg, size_t size)
{
    (void)userdata; (void)msg; (void)size;
    // サポートされていません
}


/* NonStandard calls */
static void rtDeviceSwitch(void *userdata, size_t track, const char *data, size_t length)
{
    (void)userdata; (void)track; (void)data; (void)length;
    // サポートされていません
}

static size_t rtCurrentDevice(void *userdata, size_t track)
{
    (void)userdata; (void)track;
    // サポートされていません
    return 0;
}
/* NonStandard calls End */

static void playSynth(void *userdata, uint8_t *stream, size_t length)
{
    FluidMidiSeq &m = getModule(userdata);
    tsf_render_short(m.getSynth(),
                     reinterpret_cast<short*>(stream),
                     static_cast<int>(length) / 4, 0);
}


void FluidMidiSeq::initSequencerInterface()
{
    m_sequencer = new MidiSequencer;
    BW_MidiRtInterface *seq = new BW_MidiRtInterface;
    if(m_sequencerInterface)
        delete m_sequencerInterface;
    m_sequencerInterface = seq;
    std::memset(seq, 0, sizeof(BW_MidiRtInterface));

    /* MIDIリアルタイムコール */
    seq->rtUserData = this;
    seq->rt_noteOn  = rtNoteOn;
    seq->rt_noteOff = rtNoteOff;
    seq->rt_noteAfterTouch = rtNoteAfterTouch;
    seq->rt_channelAfterTouch = rtChannelAfterTouch;
    seq->rt_controllerChange = rtControllerChange;
    seq->rt_patchChange = rtPatchChange;
    seq->rt_pitchBend = rtPitchBend;
    seq->rt_systemExclusive = rtSysEx;

    seq->onPcmRender = playSynth;
    seq->onPcmRender_userData = this;

    seq->pcmSampleRate = m_rate;
    seq->pcmFrameSize = 2 /*channels*/ * 2 /*size of one sample*/;

    /* 非標準コール */
    seq->rt_deviceSwitch = rtDeviceSwitch;
    seq->rt_currentDevice = rtCurrentDevice;

    m_sequencer->setInterface(seq);
    m_sequencer->setDeviceMask(BW_MidiSequencer::Device_GeneralMidi|BW_MidiSequencer::Device_SoundMasterII|BW_MidiSequencer::Device_GravisUltrasound);
}

void FluidMidiSeq::initFluid(const char *bankPath)
{
    closeFluid();
    synth = tsf_load_filename(bankPath);
//    synth = tsf_load_filename("/home/wohlstand/Yandex.Disk/Timidity-SNES/timidity/SNES.sf2");
    for(int ch = 0; ch < 16; ch++)
        tsf_channel_set_bank(synth, ch, 0);
    tsf_channel_set_bank_preset(synth, 9, 128, 0);
    tsf_set_output(synth, TSF_STEREO_INTERLEAVED, static_cast<int>(m_rate), -2.0f);

    for(int i = 0; i < synth->presetNum; ++i)
    {
        struct tsf_preset &p = synth->presets[i];
        printf("Preset %d [%d:%d]: %s\n", i, p.bank, p.preset, p.presetName);

        for(int j = 0; j < p.regionNum; ++j)
        {
            struct tsf_region &r = p.regions[j];
            printf("-- region %d: [%d...%d]\n", j, r.lokey, r.hikey);
        }
    }
}

void FluidMidiSeq::closeFluid()
{
    if(synth)
    {
        tsf_close(synth);
        synth = nullptr;
    }
}

FluidMidiSeq::FluidMidiSeq(const char *bankPath, uint32_t rate)
{
    synth = nullptr;

    m_sequencer = nullptr;
    m_sequencerInterface = nullptr;
    m_rate = rate;

    initFluid(bankPath);
    initSequencerInterface();
}

FluidMidiSeq::~FluidMidiSeq()
{
    if(m_sequencer)
        delete m_sequencer;
    if(m_sequencerInterface)
        delete m_sequencerInterface;
    closeFluid();
}

tsf *FluidMidiSeq::getSynth()
{
    return synth;
}

void FluidMidiSeq::setLoop(bool enable)
{
    m_sequencer->setLoopEnabled(enable);
}

bool FluidMidiSeq::Open(char *filename)
{
    return m_sequencer->loadMIDI(filename);
}

bool FluidMidiSeq::Load(const void *buf, size_t size)
{
    return m_sequencer->loadMIDI(buf, size);
}

double FluidMidiSeq::Tick(double s, double granularity)
{
    MidiSequencer &seqr = *m_sequencer;
    return seqr.Tick(s, granularity);
}

int FluidMidiSeq::playBuffer(uint8_t *stream, int len)
{
    return m_sequencer->playStream(stream, static_cast<size_t>(len));
}
