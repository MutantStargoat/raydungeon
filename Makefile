src = $(wildcard src/*.c)
obj = $(src:.c=.o)
dep = $(src:.c=.d)
bin = game

warn = -pedantic -Wall
dbg = -g

CFLAGS = $(warn) $(opt) $(dbg) $(inc) $(def) -MMD
LDFLAGS = $(syslib) $(libgl)

sys := $(shell uname -s | sed 's/MINGW.*/mingw/')
ifeq ($(sys), mingw)
	syslib = -mwindows
	libgl = -lopengl32 -lfreeglut

	obj = $(src:.c=.w32.o)
	dep = $(src:.c=.w32.d)
	bin = game.exe
else
	libgl = -lGL -lGLU -lglut
endif

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

%.w32.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cross
cross:
	$(MAKE) CC=i686-w64-mingw32-gcc sys=mingw

.PHONY: cross-clean
cross-clean:
	$(MAKE) sys=mingw clean
