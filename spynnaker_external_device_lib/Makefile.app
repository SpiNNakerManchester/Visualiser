OBJS=$(OBJECTS:%=build/%)

MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(dir $(MAKEFILE_PATH))
UNAME_S := $(shell uname -s)

ifeq ($(findstring MINGW, $(UNAME_S)), MINGW)
    CPPFLAGS += -D WINVER=0x501 -D WIN32_LEAN_AND_MEAN -fpermissive 
    PRE_LDFLAGS += -static-libgcc -static-libstdc++
    LIBS += -lspynnaker_external_device_lib -lpthread -lws2_32 -lsqlite3
endif
ifeq ($(UNAME_S), Darwin)
    LIBS += -lspynnaker_external_device_lib -lc++ -lpthread -lsqlite3
endif
ifeq ($(UNAME_S), Linux)
    LIBS += -lspynnaker_external_device_lib -lpthread -lsqlite3
endif


all: $(APP)

build/%.o: %.cpp
	-mkdir -p $(dir $@)
	$(CXX) -I$(CURRENT_DIR) $(CPPFLAGS) -o $@ -c $<

$(APP): $(OBJS)
	$(CXX) $(PRE_LDFLAGS) -L$(CURRENT_DIR) -o $@ $^ $(LIBS)

clean:
	rm -rf build
