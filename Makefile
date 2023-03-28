src = $(wildcard src/*.c)
obj = $(src:.c=.o)
dep = $(src:.c=.d)
bin = game

warn = -pedantic -Wall
dbg = -g
def = -DMINIGLUT_USE_LIBC

inc = -Ilibs/assfile
liblist = libs/assfile/assfile.a

CFLAGS = $(warn) $(opt) $(dbg) $(inc) $(def) -MMD
LDFLAGS = $(syslib) $(libgl) $(libs) -lm

sys := $(shell uname -s | sed 's/MINGW.*/mingw/')
ifeq ($(sys), mingw)
	syslib = -mwindows
	libgl = -lopengl32 -lgdi32 -lwinmm

	obj = $(src:.c=.w32.o)
	dep = $(src:.c=.w32.d)
	bin = game.exe
	libs = $(liblist:.a=.w32.a)
else
	libgl = -lGL -lGLU -lX11
	libs = $(liblist)
endif

$(bin): $(obj) libs
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

%.w32.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@

.PHONY: libs
libs:
	$(MAKE) -C libs/assfile

.PHONY: clean-libs
clean-libs:
	$(MAKE) -C libs/assfile clean

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cross
cross:
	$(MAKE) CC=i686-w64-mingw32-gcc AR=i686-w64-mingw32-ar sys=mingw

.PHONY: clean-cross
clean-cross:
	$(MAKE) sys=mingw clean
