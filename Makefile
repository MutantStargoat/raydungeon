src = $(wildcard src/*.c) libs/glew/glew.c
obj = $(src:.c=.o)
dep = $(src:.c=.d)
bin = game

warn = -pedantic -Wall
dbg = -g
def = -DMINIGLUT_USE_LIBC -DGLEW_STATIC

inc = -Ilibs -Ilibs/assfile -Ilibs/treestor/include -Ilibs/glew
libs = -lassfile -ldrawtext -lgoat3d -limago -ltreestor

CFLAGS = $(warn) $(opt) $(dbg) $(inc) $(def) -MMD
LDFLAGS = $(libdir) $(syslib) $(libgl) $(libs) -lm

sys := $(shell uname -s | sed 's/MINGW.*/mingw/')
ifeq ($(sys), mingw)
	syslib = -mwindows
	libgl = -lopengl32 -lgdi32 -lwinmm

	obj = $(src:.c=.w32.o)
	dep = $(src:.c=.w32.d)
	bin = game.exe

	libdir = -Llibs/w32
else
	libgl = -lGL -lX11

	libdir = -Llibs/unix
endif

$(bin): $(obj) libs
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

%.w32.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@

.PHONY: libs
libs:
	$(MAKE) -C libs

.PHONY: clean-libs
clean-libs:
	$(MAKE) -C libs clean

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cross
cross:
	$(MAKE) CC=i686-w64-mingw32-gcc AR=i686-w64-mingw32-ar sys=mingw

.PHONY: clean-cross
clean-cross:
	$(MAKE) sys=mingw clean
