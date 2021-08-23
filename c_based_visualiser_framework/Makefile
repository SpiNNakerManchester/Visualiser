# Copyright (c) 2015-2021 The University of Manchester
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
