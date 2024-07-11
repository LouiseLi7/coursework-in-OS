target:=  libfs.a
objs    := disk.o fs.o
CC      := gcc
CFLAGS  := -Wall -Wextra -Werror -MMD

ifneq ($(V),1)
Q = @
endif

all:$(target)

deps := $(patsubst %.o, %.d, $(objs))
-include $(deps)

$(target): $(objs)
	@echo "CC $@ "
	ar rcs $(target)  $@ $^

%.o: %.c 
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo "CLEAN"	
	$(Q)rm -f $(targets) $(objs) $(deps)
