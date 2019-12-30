/* FIXME: add license block */
#ifndef LJMUSIC_H
#define LJMUSIC_H

#define LJMUSIC_USING_DUMB 1
#define LJMUSIC_USING_VORBIS 1

typedef struct LJMusic LJMusic;

struct LJMusic *LJMusic_new(void);
void LJMusic_delete(struct LJMusic *m);
int LJMusic_load(struct LJMusic *m, const char *filename);
void LJMusic_unload(struct LJMusic *m);
void LJMusic_start(struct LJMusic *m, int bufferSize, int vol);
void LJMusic_stop(struct LJMusic *m);
void LJMusic_poll(struct LJMusic *m);
void LJMusic_pause(struct LJMusic *m, int value);


#endif

