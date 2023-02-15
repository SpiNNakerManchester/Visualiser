# Copyright (c) 2015 The University of Manchester
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
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
