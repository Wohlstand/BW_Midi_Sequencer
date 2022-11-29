#include <stdint.h>
#include <stddef.h>

typedef struct tsf tsf;

// クラスの名前を変更してABIの衝突を回避する
#define BW_MidiSequencer FluidMidiSequencer
class BW_MidiSequencer;
typedef BW_MidiSequencer MidiSequencer;
typedef struct BW_MidiRtInterface BW_MidiRtInterface;

class FluidMidiSeq
{
    tsf *synth;

    uint32_t m_rate;

    MidiSequencer *m_sequencer;
    BW_MidiRtInterface *m_sequencerInterface;
    void initSequencerInterface();
    void initFluid(const char *bankPath);
    void closeFluid();
public:
    explicit FluidMidiSeq(const char *bankPath, uint32_t rate = 44100);
    ~FluidMidiSeq();
    tsf *getSynth();
    void setLoop(bool enable);
    bool Open(char *filename);
    bool Load(const void *buf, size_t size);
    double Tick(double s, double granularity);
    int playBuffer(uint8_t *stream, int len);
};
