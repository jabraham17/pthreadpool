CC= clang

# -Wpedantic is just annoying
# -Weverything is an option, but its a little too much
override CFLAGS+= -Wall -Wextra -O3 -pthread #-fopenmp

ifeq ($(DEBUG),1)
override CFLAGS+= -DDEBUG -g
endif

ifeq ($(ACCURACY),1)
override CFLAGS+= -DACCURACY
endif

ifdef ($(THREADS))
override CFLAGS+= -DNUM_THREADS=$(THREADS)
endif

ifeq ($(VECTORIZE),1)
override CFLAGS+= -DVECTORIZE -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -msse4 -mavx -mavx2 -mfma
endif

override LDFLAGS+= $(CFLAGS)
override LDLIBS+=

SOURCES= $(wildcard *.c)
OBJECTS= $(patsubst %.c,%.o,$(SOURCES))

# determine .h dependencies for headers
DEPENFLAGS+= -MMD -MP
DEPENDS= $(patsubst %.c,%.d,$(SOURCES))
INCLUDE= 

TARGET= test.out


.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJECTS) $(DEPENDS)


$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(LDLIBS) $^ -o $@

# depend on makefile to update when Makefile changes
%.o: %.c Makefile
	$(CC) $(CFLAGS) $(DEPENFLAGS) $(INCLUDE) -c $< -o $@

# include dependencies
-include $(DEPENDS)
