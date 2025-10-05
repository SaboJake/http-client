CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -I.
LDFLAGS =

SOURCES = client.cpp requests.cpp helpers.cpp commands.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = client

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

run: $(EXECUTABLE)
	./$(EXECUTABLE)

.PHONY: all clean run