# tools
CC := gcc
RM := rm -f
EXECSTACK := execstack

# flags
CFLAGS := -ggdb -m32 -g -std=c99 -D_GNU_SOURCE -fno-stack-protector -mpreferred-stack-boundary=2 -Wno-format-security
LDFLAGS := -m32
LDLIBS :=

# sources
sources := vul1.c vul2.c vul3.c vul4.c vul5.c vul6.c extra-credit.c tmalloc.c
targets := vul1 vul2 vul3 vul4 vul5 vul6 extra-credit

# gmake magic
.PHONY: default all clean

#targets
default: all
all: mkexecstack

mkexecstack: $(targets)
	$(EXECSTACK) -s $(targets)

install: mkexecstack
	install -o root -t /tmp $(targets)
	chmod 4755 /tmp/vul*

%.bin: %.o
	objcopy -S -O binary -j .text $< $@

%.o: %.c
	$(CC) $< -c -o $@ $(CFLAGS)

extra-credit.o: extra-credit.c
	$(CC) $< -c -o $@ -fstack-protector-all -ggdb -m32 -g -std=c99 -D_GNU_SOURCE

%.o: %.S
	$(CC) $< -c -o $@ $(CFLAGS)

clean:
	$(RM) $(targets) $(sources:.c=.o)

#dependencies
vul4: tmalloc.o
tmalloc.o vul4.o: tmalloc.h
