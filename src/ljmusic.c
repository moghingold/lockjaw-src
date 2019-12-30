/* FIXME: add license block */
#include <allegro.h>
#include "ljmusic.h"

#if LJMUSIC_USING_DUMB
#include <aldumb.h>
static int dumb_inited;
#endif
#if LJMUSIC_USING_VORBIS
#include "ljvorbis.h"
#endif

struct LJMusic {
  int paused;
#if LJMUSIC_USING_VORBIS
  LJVorbis *ogg;
#endif
#if LJMUSIC_USING_DUMB
  DUH *duh;
  AL_DUH_PLAYER *duhplayer;
#endif
};

struct LJMusic *LJMusic_new(void) {
  struct LJMusic *m = malloc(sizeof(struct LJMusic));
  if (!m) {
    return NULL;
  }
#if LJMUSIC_USING_VORBIS
  m->ogg = NULL;
#endif
#if LJMUSIC_USING_DUMB
  m->duh = NULL;
  m->duhplayer = NULL;
#endif
  return m;
}

void LJMusic_delete(struct LJMusic *m) {
  if (!m) {
    return;
  }
  LJMusic_unload(m);
  free(m);
}

int LJMusic_load(struct LJMusic *m, const char *filename) {
  if (!m || !filename) {
    return -1;
  }

  LJMusic_unload(m);
  const char *ext = get_extension(filename);

#if LJMUSIC_USING_VORBIS
  if (!ustricmp(ext, "ogg")) {
    m->ogg = LJVorbis_open(filename);
    return m->ogg ? 1 : 0;
  }
#endif

#if LJMUSIC_USING_DUMB
  if (!dumb_inited) {
    atexit(dumb_exit);
    dumb_register_stdfiles();
  }
  if (!ustricmp(ext, "it")) {
    m->duh = dumb_load_it_quick(filename);
    return m->duh ? 1 : 0;
  }
  if (!ustricmp(ext, "xm")) {
    m->duh = dumb_load_xm_quick(filename);
    return m->duh ? 1 : 0;
  }
  if (!ustricmp(ext, "s3m")) {
    m->duh = dumb_load_s3m_quick(filename);
    return m->duh ? 1 : 0;
  }
  if (!ustricmp(ext, "mod")) {
    m->duh = dumb_load_mod_quick(filename);
    return m->duh ? 1 : 0;
  }
#endif

  return 0;
}

void LJMusic_unload(struct LJMusic *m) {
  if (!m) {
    return;
  }
  LJMusic_stop(m);

#if LJMUSIC_USING_DUMB
  if (m->duh) {
    unload_duh(m->duh);
    m->duh = NULL;
  }
#endif

#if LJMUSIC_USING_VORBIS
  if (m->ogg) {
    LJVorbis_close(m->ogg);
    m->ogg = NULL;
  }
#endif

}

void LJMusic_start(struct LJMusic *m, int bufferSize, int vol) {
  if (!m) {
    return;
  }

  LJMusic_stop(m);

#if LJMUSIC_USING_DUMB
  if (m->duh) {
    m->duhplayer = al_start_duh(m->duh, 2, 0,
                                vol / 256.0,
                                bufferSize,
                                48000);
  }
#endif
#if LJMUSIC_USING_VORBIS
  if (m->ogg) {
    LJVorbis_start(m->ogg, bufferSize, vol, 128);
  }
#endif
  m->paused = 0;
}

void LJMusic_stop(struct LJMusic *m) {
  if (!m) {
    return;
  }

#if LJMUSIC_USING_DUMB
  if (m->duhplayer) {
    al_stop_duh(m->duhplayer);
    m->duhplayer = NULL;
  }
#endif

#if LJMUSIC_USING_VORBIS
  LJVorbis_stop(m->ogg);
#endif
}

void LJMusic_poll(struct LJMusic *m) {
  if (!m) {
    return;
  }

#if LJMUSIC_USING_DUMB
  if (m->duhplayer) {
    al_poll_duh(m->duhplayer);
  }
#endif

#if LJMUSIC_USING_VORBIS
  if (m->ogg) {
    LJVorbis_poll(m->ogg);
  }
#endif
}

void LJMusic_pause(struct LJMusic *m, int value) {
  value = value ? 1 : 0;
  if (!m) {

  }
  if (value == m->paused) {
    alert("LJMusic_pause", "unchanging.", "", "OK", 0, 13, 0);
    return;
  }
#if LJMUSIC_USING_DUMB
  if (m->duhplayer) {
    if (value) {
      al_pause_duh(m->duhplayer);
    } else {
      al_resume_duh(m->duhplayer);
    }
  }
#endif

#if LJMUSIC_USING_VORBIS
  if (m->ogg) {
    LJVorbis_pause(m->ogg, value);
  }
#endif
  m->paused = value;
}

