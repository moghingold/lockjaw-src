# Makefile for Allegro version of LOCKJAW
#
# Copr. 2006-2007 Damian Yerrick
# 
# This work is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


EXE := lj.exe
CFLAGS := -Wall -O2 -std=gnu99 -DWITH_REPLAY=1
CC := gcc
LD := gcc
LDFLAGS := -Wall -s -mwindows
srcdir := src
objdir := obj/win32

MUSICOBJS := $(objdir)/ljvorbis.o 
MUSICLIBS := -laldmb -ldumb -lvorbisfile -lvorbis -logg

LDLIBS := -ljpgal -lalleg
DEPOBJS := $(objdir)/ljpc.o $(objdir)/lj.o $(objdir)/ljplay.o $(objdir)/pcjoy.o $(objdir)/gimmicks.o $(objdir)/wktables.o $(objdir)/options.o $(objdir)/debrief.o $(objdir)/macro.o $(objdir)/ljreplay.o $(objdir)/ljmusic.o $(MUSICOBJS)

OTHEROBJS := $(objdir)/winicon.o

.PHONY: clean lj.gba lj.nds

# Objects

$(EXE): $(DEPOBJS) $(OTHEROBJS)
	$(LD) $(LDFLAGS) $^ $(MUSICLIBS) $(LDLIBS) -o $@

all: $(EXE) lj.gba lj.nds

lj.gba:
	make -f gbamakefile $@

lj.nds:
	make -f dsmakefile $@

# Compilation rules

$(objdir)/%.o: $(srcdir)/%.c
	$(CC) $(CFLAGS) -MMD -c -o $@ $<
	@cp $(objdir)/$*.d $(objdir)/$*.P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' < $(objdir)/$*.d >> $(objdir)/$*.P; \
	rm -f $(objdir)/$*.d

$(objdir)/%.o: src/%.rc docs/favicon.ico
	windres -i $< -o $@

# Header dependencies

-include $(DEPOBJS:%.o=%.P)


# Cleanup rules

clean:
	-rm $(objdir)/*.o
	-rm $(objdir)/*.P
