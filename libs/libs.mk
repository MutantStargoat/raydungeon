# included from a makefile which defines libname, src

obj = $(src:.c=.o)
lib = $(libname).a

CFLAGS = -pedantic -Wall -g

ifeq ($(sys), mingw)
	obj = $(src:.c=.w32.o)
	lib = $(libname).w32.a
endif

$(lib): $(obj)
	$(AR) rcs $@ $(obj)

%.w32.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@

.PHONY: clean
clean:
	rm -f $(obj) $(lib)
