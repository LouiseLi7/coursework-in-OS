target:  libuthread.a
objs    := uthread.o context.o queue.o preempt.o 
CC      := gcc
CFLAGS  := -Wall -Wextra -Werror -MMD
CFLAGS  += -g
PANDOC  := pandoc

ifneq ($(V),1)
Q = @
endif


deps := $(patsubst %.o, %.d, $(objs))
-include $(deps)

libuthread.a: $(objs)
	@echo "CC $@ "
	ar rcs libuthread.a  $@ $^

%.o: %.c 
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo "CLEAN"
	rm -f $(targets) $(objs)
	$(Q)rm -f $(targets) $(objs) $(deps)