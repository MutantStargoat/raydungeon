# included from a makefile which defines libname, src

obj = $(src:.c=.o)
lib = ../unix/lib$(libname).a

CFLAGS = -O3 $(inc) $(def)

ifeq ($(sys), mingw)
	obj = $(src:.c=.w32.o)
	lib = ../w32/lib$(libname).a
endif

$(lib): $(obj)
	$(AR) rcs $@ $(obj)

%.w32.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@

.PHONY: clean
clean:
	rm -f $(obj) $(lib)
