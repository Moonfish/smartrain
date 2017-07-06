# makefile for smartrain
VPATH = src
OBJDIR = obj
INC=-I/home/russell/code/cpp/boost_1_64_0
LIB=-L/home/russell/code/arm/boost/bin.v2/libs/system/build/gcc-4.9.3/release/threading-multi/CXX=/home/russell/code/arm/raspi/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++
  
CFLAGS=-Wall -std=c++11 -Wl,--no-as-needed $(INC) $(LIB) 
LDFLAGS=-Wl,--no-as-needed -pthread -lboost_system
OBJECTS=$(addprefix $(OBJDIR)/, BoneIO.o util.o MCP23008.o Keypad.o LCD.o \
        RelayController.o MessageQueue.o States.o SmartRain.o NetworkInfo.o \
	WeatherService.o json11.o EventLogger.o )

EXECUTABLE=smartrain

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LIB) $(LDFLAGS) -o $(EXECUTABLE) $(OBJECTS)

# for any .o, compile the correspoinding .cpp in he src dir.
$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJECTS)l
	$(RM) $(EXECUTABLE)


