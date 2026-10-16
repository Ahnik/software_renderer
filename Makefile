CXX      := clang++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -Wshadow -O3 -MMD -MP

TARGET   := renderer
SRCS     := main.cpp model.cpp tgaimage.cpp
OBJS     := $(SRCS:.cpp=.o)
DEPS     := $(OBJS:.o=.d)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) obj/diablo3_pose/diablo3_pose.obj

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET) framebuffer.tga

-include $(DEPS)
