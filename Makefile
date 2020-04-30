all: dedup

CXX := g++

CXX_SRCS = $(wildcard *.cc)
CXX_OBJS := $(patsubst %.cc,%.o,$(CXX_SRCS))
CXX_DEPS := $(patsubst %.cc,%.d,$(CXX_SRCS))

CXX_INCLUDE := -I.
CXX_WARNINGS=-pedantic -Wall -Wextra -Wcast-align -Wcast-qual \
	-Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self \
	-Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept \
	-Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow \
	-Wsign-conversion -Wsign-promo -Wstrict-null-sentinel \
	-Wstrict-overflow=5 -Wswitch-default -Wundef -Werror -Wno-unused \
	-Wall -Wextra -Wformat-nonliteral -Wcast-align -Wpointer-arith \
	-Wmissing-declarations -Winline -Wundef -Wcast-qual \
	-Wshadow -Wwrite-strings -Wno-unused-parameter -Wfloat-equal \
	-pedantic
# O2
CXX_FLAGS := -g3 -std=c++14

%.o: %.cc $(CXX_HEADERS) Makefile
	g++ $(CXX_FLAGS) $(CXX_WARNINGS) $(CXX_INCLUDE) -MMD -MP -c -o $@ $<

dedup: $(CXX_OBJS)
	g++ $(CXX_FLAGS) $(CXX_INCLUDE) -o $@ $^

all: dedup

.PHONY: all clean

-include $(CXX_DEPS)

clean:
	rm dedup $(CXX_OBJS) $(CXX_DEPS)
