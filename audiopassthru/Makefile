CXX = g++
CXXFLAGS = -std=c++17 -Iinclude -I../dsp/include -I../dsp/ptutil/include -Wall -Wno-narrowing -Wno-write-strings -DDSPSOFT_TARGET -fPIC

SRCS = \
	src/AudioPassthru/AudioPassthru.cpp \
	src/AudioPassthru/AudioPassthruLinux.cpp \
	src/AudioPassthru/LinuxShim.cpp \
	src/MTH/MthBuffer.cpp

OBJS = $(SRCS:.cpp=.o)

libaudiopassthru.a: $(OBJS)
	ar rcs $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) libaudiopassthru.a
