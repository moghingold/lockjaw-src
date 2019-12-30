/*

ljvorbis.c
Simple wrapper around vorbisfile for use with the Allegro library
based on vorbisfile_example.c

Copyright 2006 Damian Yerrick
Copyright 2002-2004 Xiph.org Foundation

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

- Neither the name of the Xiph.org Foundation nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#define ALLEGRO_USE_CONSOLE
#include <stdio.h>
#include "ljvorbis.h"

#define VORBIS_BYTEDEPTH 2  // number of bytes per sample; 2 means 16-bit
#define VORBIS_ENDIAN 0  // 0: x86/little; 2: ppc/big
#define VORBIS_SIGNED 0  // 0: unsigned; 1: signed; Allegro uses unsigned

LJVorbis *LJVorbis_open(const char *filename) {
  LJVorbis *ogg = malloc(sizeof(LJVorbis));
  if (!ogg) {
    return NULL;
  }
  ogg->fp = fopen(filename, "rb");
  if (!ogg->fp) {
    free(ogg);
    return NULL;
  }
  if (ov_open(ogg->fp, &(ogg->vf), NULL, 0) < 0) {
    fclose(ogg->fp);
    free(ogg);
    return NULL;
  }
  vorbis_info *vi=ov_info(&(ogg->vf),-1);
  ogg->rate = vi->rate;
  ogg->channels = vi->channels;
  ogg->length = ov_pcm_total(&(ogg->vf),-1);
  ogg->voice = NULL;
  ogg->paused = 0;
  return ogg;
}

int LJVorbis_start(LJVorbis *ogg, int bufferSize, int vol, int pan) {
  if (ogg) {
  
    // if restarting, stop first
    if (ogg->voice) {
      LJVorbis_stop(ogg);
    }
    ogg->voice = play_audio_stream(bufferSize,
                                   8 * VORBIS_BYTEDEPTH,
                                   ogg->channels > 1,
                                   ogg->rate,
                                   vol, pan);
    ogg->bufferSize = bufferSize;
    if (!ogg->voice) {
      return -1;
    }
    ov_pcm_seek(&(ogg->vf), 0);
    return 0;
  }
  return -1;
}

void LJVorbis_stop(LJVorbis *ogg) {
  if (ogg) {
    if (ogg->voice) {
      stop_audio_stream(ogg->voice);
      ogg->voice = NULL;
    }
  }
}

void LJVorbis_close(LJVorbis *ogg) {
  if (ogg) {
    LJVorbis_stop(ogg);
    ov_clear(&(ogg->vf));
    if (ogg->fp) {
      fclose(ogg->fp);
    }
    free(ogg);
  }
}

void LJVorbis_pause(LJVorbis *ogg, int value) {
  if (ogg && ogg->voice) {
    int hwVoice = ogg->voice->voice;
    voice_set_frequency(hwVoice, value ? 0 : ogg->rate);
    ogg->paused = value ? 1 : 0;
  }
}

int LJVorbis_poll(LJVorbis *ogg) {
  if (!ogg) {
    return -1;
  }
  char *buf = get_audio_stream_buffer(ogg->voice);
  int eofReached = 0;
    
  if (buf) {
    // the number of bytes left in this buffer
    long int bytesLeft = ogg->bufferSize * VORBIS_BYTEDEPTH * ogg->channels;

    while (bytesLeft > 0) {
      long ret=ov_read(&(ogg->vf),
                       buf,
                       bytesLeft,
                       VORBIS_ENDIAN,
                       VORBIS_BYTEDEPTH,
                       VORBIS_SIGNED,
                       &(ogg->bitstream));
      if (ret == 0) {
        // try to seek back to the beginning of the file
        int pcmErr = ov_pcm_seek(&(ogg->vf), 0);
        if (pcmErr) {
          /* EOF */
          eofReached = 1;
          bytesLeft = 0;
        }
      } else if (ret < 0) {
        // Stream error. Just ignore it.
      } else {
        /* FIXME: handle sample rate changes, etc */
        // advance forward in the buffer
        buf += ret;
        bytesLeft -= ret;
      }
    }
    free_audio_stream_buffer(ogg->voice);
  }
  return eofReached;
}

#ifdef LJVORBIS_DEMO
int main(){
  int eofReached = 0;
  LJVorbis *ogg = LJVorbis_open("ozma.ogg");

  if (!ogg) {
    fprintf(stderr, "Could not open ozma.ogg.\n");
    exit(1);
  }
  if (allegro_init() < 0
      || install_timer() < 0
      || set_gfx_mode(GFX_SAFE, 320, 200, 0, 0) < 0
      || install_keyboard() < 0
      || install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL) < 0) {
    allegro_exit();
    LJVorbis_close(ogg);
    fprintf(stderr, "Could not start Allegro library: %s\n", allegro_error);
    exit(1);
  }

  /* Throw the comments plus a few lines about the bitstream we're
     decoding */
  if (0)
  {
    char line1[80], line2[80];
    
    usprintf(line1,"%d channels, %u Hz", ogg->channels, ogg->rate);
    usprintf(line2,"length: %lu samples", ogg->length);
    alert(line1, line2, "ready?",
          "Play", 0, 13, 0);
  }
  
  if (LJVorbis_start(ogg, 1024, 192, 128) < 0) {
    LJVorbis_close(ogg);
    alert("Could not allocate voice",
          "for playing audio.",
          "",
          "OK", 0, 13, 0);
    exit(1);
  }
  
  while(!eofReached){
    eofReached = LJVorbis_poll(ogg);
    rest(16);
    
    if (keypressed()) {
      int scancode;
      ureadkey(&scancode);
      
      if (scancode == KEY_P) {
        LJVorbis_pause(ogg, !ogg->paused);
      } else if (scancode == KEY_ESC) {
        eofReached = 1;
      }
    }
  }

  /* cleanup */
  LJVorbis_close(ogg);
  return(0);
}
END_OF_MAIN();
#endif
