# get the system name
SYSTEM = $(shell uname)

# change this in shell to -g for debug
OPT_FLAGS ?= -O3

# change this in shell 

ifeq ($(SYSTEM),IRIX64)
# IRIX 6.x with new 32-bit libraries
CXX	  = CC -LANG:std
CXXFLAGS    = -fullwarn -n32 -ptused $(OPT_FLAGS)
INC_DIRS    = -I/usr/local/include

OPENGL_LIBS = -L/usr/share/src/OpenGL/toolkits/lib32  \
	-lglui -lglut -lGLU -lGL -lX11 -lXmu
OPENGL_INC = -I/usr/include/GL -I/usr/share/src/OpenGL/toolkits/include/GL \
	-I/usr/share/src/OpenGL/toolkits/include 
endif

ifeq ($(SYSTEM),Linux)
CXX	  = g++
CXXFLAGS    = -Wall -fexceptions $(OPT_FLAGS) 
# extras for Athlon-MP system
ifeq ($(HOST),linux-huwis.lboro.ac.uk)
CXXFLAGS += -funroll-loops -fomit-frame-pointer -ffast-math -march=athlon-mp -mfpmath=sse
endif

OPENGL_LIBS = -L/usr/X11R6/lib  \
	-lglui -lglut -lGLU -lGL -lXmu -lXi -lX11
OPENGL_INC = -I/usr/X11R6/include -I/usr/X11R6/include/GL \
        -I/usr/include/GL 
endif

ifeq ($(SYSTEM),Darwin)
CXX	  = c++
CXXFLAGS    = -Wall -fexceptions $(OPT_FLAGS) 
# suggested by linker
LDFLAGS = -Xlinker -bind_at_load

OPENGL_LIBS = -L$(GLUT_DIR)/lib -L/usr/X11R6/lib \
	-lglui -lglut -lGLU -lGL -lXmu -lXext -lX11
OPENGL_INC = -I$(GLUT_DIR)/include -I$(GLUT_DIR)/include/GL \
	-I/usr/X11R6/include -I/usr/X11R6/include/GL 
GLUT_DIR = /usr/local/
endif

ifeq ($(SYSTEM),CYGWIN_NT-5.1)
# GNU compiler flags (works with g++ and egcs) - Darwin version
CXX	  = c++
CXXFLAGS    = -Wall -fexceptions $(OPT_FLAGS) 

OPENGL_LIBS = -L/usr/X11R6/lib \
	-lglui -lglut -lGLU -lGL -lXmu -lXext -lX11
OPENGL_INC = -I/usr/include/GL \
	-I/usr/X11R6/include -I/usr/X11R6/include 
endif

all: libglui.a 

GLUI_OBJS = glui_add_controls.o glui.o glui_bitmap_img_data.o \
glui_bitmaps.o glui_button.o glui_edittext.o glui_checkbox.o \
glui_node.o glui_radio.o glui_statictext.o glui_panel.o \
glui_separator.o glui_spinner.o glui_control.o glui_column.o \
glui_translation.o glui_rotation.o glui_mouse_iaction.o \
glui_listbox.o glui_rollout.o arcball.o algebra3.o quaternion.o

libglui.a: $(GLUI_OBJS)
	@echo "Creating library"
	ar -r libglui.a $(GLUI_OBJS)
	ranlib libglui.a

%.o: %.cpp
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $(OPENGL_INC) $< -o $@

%.o: %.c
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $(OPENGL_INC) $< -o $@

clean:
	/bin/rm -f *~ *.bak *.bck *.tmp *.o
	/bin/rm -f libglui.a



algebra3.o: algebra3.h
arcball.o: arcball.h algebra3.h quaternion.h stdinc.h
glui.o: glui.h  stdinc.h 
glui_add_controls.o: glui.h  stdinc.h 
glui_bitmap_img_data.o: glui_img_checkbox_0.c glui_img_checkbox_1.c
glui_bitmap_img_data.o: glui_img_radiobutton_0.c glui_img_radiobutton_1.c
glui_bitmap_img_data.o: glui_img_uparrow.c glui_img_downarrow.c
glui_bitmap_img_data.o: glui_img_leftarrow.c glui_img_rightarrow.c
glui_bitmap_img_data.o: glui_img_spinup_1.c glui_img_spinup_0.c
glui_bitmap_img_data.o: glui_img_spindown_1.c glui_img_spindown_0.c
glui_bitmap_img_data.o: glui_img_checkbox_0_dis.c glui_img_checkbox_1_dis.c
glui_bitmap_img_data.o: glui_img_radiobutton_0_dis.c
glui_bitmap_img_data.o: glui_img_radiobutton_1_dis.c glui_img_spinup_dis.c
glui_bitmap_img_data.o: glui_img_spindown_dis.c
glui_bitmaps.o: glui.h stdinc.h 
glui_button.o: glui.h stdinc.h 
glui_checkbox.o: glui.h stdinc.h 
glui_column.o: glui.h stdinc.h 
glui_control.o: glui.h stdinc.h 
glui_edittext.o: glui.h stdinc.h 
glui_node.o: glui.h stdinc.h 
glui_panel.o: glui.h stdinc.h 
glui_radio.o: glui.h stdinc.h 
glui_separator.o: glui.h stdinc.h 
glui_spinner.o: glui.h stdinc.h 
glui_statictext.o: glui.h stdinc.h 
quaternion.o: quaternion.h algebra3.h stdinc.h
glui_translation.o: glui.h stdinc.h  
glui_rotation.o: glui.h stdinc.h  
glui_mouse_iaction.o: glui.h stdinc.h  
glui_listbox.o: glui.h stdinc.h  
glui_rollout.o: glui.h stdinc.h 
