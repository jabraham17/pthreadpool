
CC= clang


ifndef OS
OS= $(shell uname)
endif

# -Wpedantic is just annoying
# -Weverything is an option, but its a little too much
override CFLAGS+= -Wall -Wextra -O3
override CFLAGS+= -pthread 


ifeq ($(DEBUG),1)
override CFLAGS+= -DDEBUG -g
endif

override LDFLAGS+= $(CFLAGS)
override LDLIBS+=

SOURCES= $(wildcard *.c)
OBJECTS= $(patsubst %.c,%.o,$(SOURCES))

# determine .h dependencies for headers
DEPENFLAGS+= -MMD -MP
DEPENDS= $(patsubst %.c,%.d,$(SOURCES))
INCLUDE= 

TARGET= libptp.a


.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJECTS) $(DEPENDS)


$(TARGET): $(OBJECTS)
	$(AR) rcs $@ $^

# depend on makefile to update when Makefile changes
%.o: %.c Makefile
	$(CC) $(CFLAGS) $(DEPENFLAGS) $(INCLUDE) -c $< -o $@

# include dependencies
-include $(DEPENDS)
