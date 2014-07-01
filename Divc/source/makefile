CXX = g++
CXXFLAGS = -std=c++11 -Wall
LDFLAGS = -lpthread

SOURCES = cellops.cpp  celltops.cpp  IO.cpp main.cpp  Mesh.cpp  vertexops.cpp  vertextops.cpp
OBJECTS = $(patsubst %, obj/%, $(addsuffix .o, $(basename $(SOURCES))))
TARGET = divc.exe

$(TARGET) : $(OBJECTS)
	@echo Linking $(TARGET)...
	@$(CXX) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

$(OBJECTS): | obj

obj:
	@mkdir -p $@

# c++ file to object file:
obj/%.o : %.cpp
	@echo Compiling $<...
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -c $< -o $@

.PHONY: all
all: $(TARGET)
	@echo Running unit tests...
	@./$(TARGET)


# When headers get involved, pattern should be explicit.
obj/Mesh.o : Mesh.h
obj/IO.o: IO.h
obj/main.o : Mesh.h IO.h

.PHONY : clean
clean : 
	-@$(RM) -rf obj $(TARGET)
