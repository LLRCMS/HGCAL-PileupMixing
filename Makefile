#makefile 


CC   =   g++

UCFLAGS = -O0 -g3 -Wall -gstabs+ -std=c++0x 
#UCFLAGS = -O3 -Wall -gstabs+ -std=c++0x


RUCFLAGS := $(shell root-config --cflags) -I./include/ 
LIBS :=  $(shell root-config --libs) -lTreePlayer
GLIBS := $(shell root-config --glibs)

VPATH = ./src/

SRCPP = main.cpp\
	Mixer.cpp\
	Utilities.cpp\
	ParReader.cpp


	
         
#OBJCPP = $(SRCPP:.cpp=.o)
OBJCPP = $(patsubst %.cpp,obj/%.o,$(SRCPP))


all : mixing.exe obj/libDictionary_C.so

obj/%.o : %.cpp
	@echo "> compiling $*"
	@mkdir -p obj/
	@$(CC) -c $< $(UCFLAGS) $(RUCFLAGS) -o $@

mixing.exe : $(OBJCPP) 
	@echo "> linking"
	@$(CC) $^ $(ACLIBS) $(LIBS) $(GLIBS)  -o $@

clean:
	@echo "> Cleaning dictionary"
	@rm -f obj/libDictionary_C.so
	@echo "> Cleaning object files and executable"
	@rm  -f obj/*.o
	@rm -f mixing.exe

obj/libDictionary_C.so: ./include/libDictionary.C
	@echo "> Generating dictionary"
	@cd include && root -b -q libDictionary.C++
	@mv ./include/libDictionary_C.so ./obj/
