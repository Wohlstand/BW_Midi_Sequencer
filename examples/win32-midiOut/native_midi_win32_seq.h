#ifndef NATIVE_MIDI_WIN32_SEQ
#define NATIVE_MIDI_WIN32_SEQ

#ifdef __cplusplus
extern "C" {
#endif

extern void *mixer_seq_init_interface();
extern void mixer_seq_free(void *seq);

extern int mixer_seq_init_midi_out(void *seq);
extern void mixer_seq_close_midi_out(void *seq);

extern int mixer_seq_openData(void *seq, void *bytes, unsigned long len);
extern int mixer_seq_openFile(void *seq, const char *path);

extern const char *mixer_seq_meta_title(void *seq);
extern const char *mixer_seq_meta_copyright(void *seq);

extern void mixer_seq_rewind(void *seq);
extern void mixer_seq_seek(void *seq, double pos);
extern double mixer_seq_tell(void *seq);
extern double mixer_seq_length(void *seq);
extern double mixer_seq_loop_start(void *seq);
extern double mixer_seq_loop_end(void *seq);
extern int mixer_seq_at_end(void *seq);

extern double mixer_seq_get_tempo_multiplier(void *seq);
extern void mixer_seq_set_tempo_multiplier(void *seq, double tempo);
extern void mixer_seq_set_loop_enabled(void *seq, int loopEn);

extern double mixer_seq_tick(void *seq, double s, double granularity);

#ifdef __cplusplus
}
#endif

#endif /* NATIVE_MIDI_WIN32_SEQ */
