OBJECTS = UDPConnection.o \
          SpynnakerDatabaseConnection.o \
          DatabaseReader.o \
          SpynnakerLiveSpikesConnection.o \
          EIEIOMessage.o \
          ConnectionListener.o
   
OBJS=$(OBJECTS:%=build/%)

UNAME_S := $(shell uname -s)
ifeq ($(findstring MINGW, $(UNAME_S)), MINGW)
    CPPFLAGS += -D WINVER=0x501 -D WIN32_LEAN_AND_MEAN -fpermissive
endif

all: libspynnaker_external_device_lib.a

build/%.o: %.cpp
	-mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) -o $@ -c $<
	
libspynnaker_external_device_lib.a: $(OBJS)
	$(RM) $@
	$(AR) rcs $@ $(OBJS)

clean:
	rm -rf build
