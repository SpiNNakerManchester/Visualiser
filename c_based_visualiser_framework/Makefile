OBJECTS = utilities/ColourReader.o \
          raster_view/RasterPlot.o \
          glut_framework/GlutFramework.o \
          glut_framework/PerformanceTimer.o \
          glut_framework/Keyboard.o \
          main.o

APP = vis

UNAME_S := $(shell uname -s)
ifeq ($(findstring MINGW, $(UNAME_S)), MINGW)
    CPPFLAGS += -D FREEGLUT_STATIC
    LIBS += -lfreeglut_static -lopengl32 -lwinmm -lgdi32 -lglu32
endif
ifeq ($(UNAME_S), Darwin)
    LIBS += -framework OpenGL -framework GLUT
endif
ifeq ($(UNAME_S), Linux)
    LIBS += -lGL -lGLU -lglut
endif

include ../spynnaker_external_device_lib/Makefile.app
