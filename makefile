# get the system name
SYSTEM = $(shell uname)

# change this in shell to -g for debug
OPT_FLAGS ?= -O3 -ffast-math
EXTRA_FLAGS ?= 

ifeq ($(SYSTEM),IRIX64)
# IRIX 6.x with new 32-bit libraries
CXX	  = CC -LANG:std
CXXFLAGS    = -fullwarn -n32 -ptused $(OPT_FLAGS) $(EXTRA_FLAGS)
INC_DIRS    = -I$(HOME)/src/libxml2-2.6.2/include -I/usr/local/include 

OPENGL_LIBS = -L/usr/share/src/OpenGL/toolkits/lib32 -L$(GLUI_DIR) \
	-lglui -lglut -lGLU -lGL -lX11 -lXmu
OPENGL_INC = -I/usr/include/GL -I/usr/share/src/OpenGL/toolkits/include/GL \
	-I/usr/share/src/OpenGL/toolkits/include -I$(GLUI_DIR)
SOCKET_LIBS = -L/usr/local/lib -lptypes
LIBS = -L$(HOME)/src/libxml2-2.6.2/.libs
endif

ifeq ($(SYSTEM),Linux)
# GNU compiler flags 
CXX	  = g++
CXXFLAGS    = -Wall -fexceptions $(OPT_FLAGS) $(EXTRA_FLAGS)
INC_DIRS = -I${HOME}/Unix/include -I${HOME}/Unix/include/libxml2
LIBS = -L${HOME}/Unix/lib
# extras for Beowulf compilation
ifeq ($(HOSTNAME),ulgbc1.liv.ac.uk)
OPT_FLAGS += -march=k6
SOCKET_LIBS = -L$(HOME)/src/ptypes-1.8.3/lib -lptypes
LIBS = -L$(HOME)/src/libxml2-2.6.2/.libs
INC_DIRS = -I$(HOME)/src/ptypes-1.8.3/include -I$(HOME)/src/libxml2-2.6.2/include
endif
ifeq ($(HOSTNAME),microlith)
#OPT_FLAGS += -march=opteron
SOCKET_LIBS = -L$(HOME)/src/ptypes-2.0.2/lib -lptypes
LIBS = -L$(HOME)/src/libxml2-2.6.2/.libs -L$(HOME)/src/cvs-1.11.5/zlib
INC_DIRS = -I$(HOME)/src/ptypes-2.0.2/include -I$(HOME)/src/libxml2-2.6.2/include
EXTRA_FLAGS += -DUSING_64_BIT_POINTERS
endif
OPENGL_LIBS = -L/usr/X11R6/lib -L$(GLUI_DIR) \
	-lglui -lglut -lGLU -lGL -lXmu -lXi -lX11
OPENGL_INC = -I/usr/X11R6/include -I/usr/X11R6/include/GL -I/usr/include/GL -I$(GLUI_DIR)
# set the memory debugging library
# LIBS += -lefence
endif

ifeq ($(SYSTEM),SunOS)
# GNU compiler flags 
CXX	  = g++
CXXFLAGS    = -Wall -fexceptions $(OPT_FLAGS) $(EXTRA_FLAGS)
INC_DIRS    = -I$(HOME)/include
SOCKET_LIBS = -L$(HOME)/lib -lptypes -lsocket -lnsl -lrt
endif

ifeq ($(SYSTEM),Darwin)
# GNU compiler flags (works with g++ and egcs)
CXX	  = c++
CXXFLAGS    = -Wall -fexceptions $(OPT_FLAGS) $(EXTRA_FLAGS) -DUPDATE_CONTROLS_ON_IDLE -DUSE_LIBTIFF -DUSE_TIFF_LZW
# suggested by linker
LDFLAGS = -Xlinker -bind_at_load
INC_DIRS = -I/usr/local/include/libxml2
OPENGL_LIBS = -L$(GLUI_DIR) -L$(GLUT_DIR)/lib -L/usr/X11R6/lib \
	-lglui -lglut -lGLU -lGL -lXmu -lXext -lX11
OPENGL_INC = -I$(GLUI_DIR) \
	-I$(GLUT_DIR)/include -I$(GLUT_DIR)/include/GL \
	-I/usr/X11R6/include -I/usr/X11R6/include/GL 
GLUT_DIR = /usr/local
LIBS = -ltiff
# set the memory debugging library
#LIBS = -lMallocDebug
endif

ifeq ($(SYSTEM),CYGWIN_NT-5.1)
# GNU compiler flags 
CXX	  = c++
CXXFLAGS    = -Wall -fexceptions $(OPT_FLAGS) $(EXTRA_FLAGS) 
INC_DIRS = -I/usr/include/libxml2 
OPENGL_LIBS = -L$(GLUI_DIR) -L$(GLUT_DIR)/lib -L/usr/X11R6/lib \
	-lglui -lglut -lGLU -lGL -lXmu -lXext -lX11
OPENGL_INC = -I/usr/include/GL  -I$(GLUI_DIR) \
	-I/usr/X11R6/include -I/usr/X11R6/include/GL 
SOCKET_LIBS = -L/usr/local/lib -lptypes
endif

# generic library and include definitions
DYN_DIR = dynamechs_4.0pre1
GLUI_DIR = glui_v2_1_beta

DYN_LIBS = $(DYN_DIR)/dm/libdm.a 
DYN_INC = -I$(DYN_DIR)/dm
DYN_LIBS_OPENGL = $(DYN_DIR)/dm/libdm_opengl.a 

SOCKET_LIBS ?= -lptypes

LIBS += -L/usr/local/lib -lxml2 -lpthread -lm -lz

SYMCCFILES = \
CyclicDriver.cc         LoadObj.cc              MyRevoluteLink.cc       Simulation.cc \
DampedSpringStrap.cc    MAMuscle.cc             ObjectiveMain.cc        StrapForce.cc \
DataFile.cc             ModifiedContactModel.cc Random.cc               StrapForceAnchor.cc \
GLUIRoutines.cc         MyMobileBaseLink.cc     SideStabilizer.cc       Util.cc \
StepDriver.cc           DataTarget.cc           UGMMuscle.cc            TIFFWrite.cc \
MyEnvironment.cc

SYMOBJECTS = $(addsuffix .o, $(basename $(SYMCCFILES) ) )

vpath %.cc src ga merge utils

BINARIES = bin/objective bin/objective_opengl \
bin/objective_socket bin/objective_socketgl  \
bin/ga bin/ga_socket \
bin/merge \
bin/CalcPhase bin/ParseContactDebug bin/ParseMAMuscleDebug \
bin/ApplyGenome

BINARIES_NO_OPENGL = bin/objective \
bin/objective_socket  \
bin/ga bin/ga_socket \
bin/merge \
bin/CalcPhase bin/ParseContactDebug bin/ParseMAMuscleDebug \
bin/ApplyGenome

all: directories libraries binaries 

no_opengl: directories libraries binaries_no_opengl

directories: bin obj obj/obj obj/opengl obj/socket obj/socketgl \
obj/ga_obj obj/ga_socket \
obj/merge \
obj/utils

binaries: $(BINARIES)

binaries_no_opengl: $(BINARIES_NO_OPENGL)

libraries: dynamechs glui

obj/obj: 
	mkdir obj/obj

obj/opengl: 
	mkdir obj/opengl

obj/socket:
	mkdir obj/socket

obj/socketgl:
	mkdir obj/socketgl

bin:
	mkdir bin

obj:
	mkdir obj
        
obj/ga_obj: 
	mkdir obj/ga_obj

obj/ga_socket:
	mkdir obj/ga_socket

obj/merge:
	mkdir obj/merge

obj/utils:
	mkdir obj/utils
	 
obj/obj/%.o : %.cc
	$(CXX) $(CXXFLAGS) $(DYN_INC) $(INC_DIRS)  -c $< -o $@

bin/objective: $(addprefix obj/obj/, $(SYMOBJECTS) )
	$(CXX) $(LDFLAGS) -o $@ $^ $(DYN_LIBS) $(LIBS)

obj/opengl/%.o : %.cc
	$(CXX) -DUSE_OPENGL $(CXXFLAGS) $(DYN_INC) $(INC_DIRS) $(OPENGL_INC)  -c $< -o $@

bin/objective_opengl: $(addprefix obj/opengl/, $(SYMOBJECTS) )
	$(CXX) $(LDFLAGS) -o $@ $^ $(DYN_LIBS_OPENGL) $(OPENGL_LIBS) $(LIBS)

obj/socket/%.o : %.cc
	$(CXX) -DUSE_SOCKETS $(CXXFLAGS) $(DYN_INC) $(INC_DIRS)  -c $< -o $@

bin/objective_socket: $(addprefix obj/socket/, $(SYMOBJECTS) )
	$(CXX) $(LDFLAGS) -o $@ $^ $(DYN_LIBS) $(SOCKET_LIBS) $(LIBS)

obj/socketgl/%.o : %.cc
	$(CXX) -DUSE_SOCKETS -DUSE_OPENGL $(CXXFLAGS) $(DYN_INC) $(OPENGL_INC) $(INC_DIRS)  -c $< -o $@

bin/objective_socketgl: $(addprefix obj/socketgl/, $(SYMOBJECTS) )
	$(CXX) $(LDFLAGS) -o $@ $^ $(DYN_LIBS_OPENGL) $(SOCKET_LIBS) $(OPENGL_LIBS) $(LIBS)

clean:
	rm -rf obj bin
	rm -f *~ *.bak *.bck *.tmp *.o 
	( cd src; rm -f *~ *.bak *.bck *.tmp *.o )
	( cd ga; rm -f *~ *.bak *.bck *.tmp *.o )
	( cd merge; rm -f *~ *.bak *.bck *.tmp *.o )

superclean:
	rm -rf obj bin
	rm -f *~ *.bak *.bck *.tmp *.o 
	( cd src; rm -f *~ *.bak *.bck *.tmp *.o )
	( cd ga; rm -f *~ *.bak *.bck *.tmp *.o )
	( cd merge; rm -f *~ *.bak *.bck *.tmp *.o )
	( cd dynamechs_4.0pre1 ; make clean )
	( cd glui_v2_1_beta ; make clean )
	rm -rf project_builder/build project_builder/GaitSym.pbproj/*.pbxuser
	find . -name '.DS_Store' -exec rm -f {} \;
	find . -name '.gdb_history' -exec rm -f {} \;
	find . -name '.#*' -exec rm -f {} \;

dynamechs: 
	( cd dynamechs_4.0pre1 ; make all )

glui: 
	( cd glui_v2_1_beta ; make all )

GACCFILES = \
GA.cc          Genome.cc      Mating.cc      Objective.cc   Population.cc  \
Random.cc      Statistics.cc  DataFile.cc    XMLConverter.cc ExpressionParser.cc

GAOBJECTS = $(addsuffix .o, $(basename $(GACCFILES) ) )

obj/ga_socket/%.o : %.cc
	$(CXX) -Isrc -DUSE_SOCKETS $(CXXFLAGS) $(INC_DIRS)  -c $< -o $@

bin/ga_socket: $(addprefix obj/ga_socket/, $(GAOBJECTS) )
	$(CXX) -o $@ $^ $(SOCKET_LIBS) $(LIBS)

obj/ga_obj/%.o : %.cc
	$(CXX) -Isrc -DUSE_FS $(CXXFLAGS) $(INC_DIRS)  -c $< -o $@

bin/ga: $(addprefix obj/ga_obj/, $(GAOBJECTS) )
	$(CXX) -o $@ $^ $(LIBS)

MERGECCFILES = \
MergeXML.cc SimulationContainer.cc DataFile.cc

MERGEOBJECTS = $(addsuffix .o, $(basename $(MERGECCFILES) ) )

obj/merge/%.o : %.cc
	$(CXX) -Isrc $(CXXFLAGS) $(DYN_INC) $(INC_DIRS)  -c $< -o $@

bin/merge: $(addprefix obj/merge/, $(MERGEOBJECTS) )
	$(CXX) -o $@ $^ $(LIBS)

obj/utils/%.o : %.cc
	$(CXX) -Isrc -Iga $(INC_DIRS) $(CXXFLAGS) -c $< -o $@

bin/CalcPhase: $(addprefix obj/utils/, CalcPhase.o )
	$(CXX) -o $@ $^ $(LIBS)
        
bin/ParseContactDebug: $(addprefix obj/utils/, ParseContactDebug.o )
	$(CXX) -o $@ $^ $(LIBS)
        
bin/ParseMAMuscleDebug: $(addprefix obj/utils/, ParseMAMuscleDebug.o )
	$(CXX) -o $@ $^ $(LIBS)
        
bin/ApplyGenome: $(addprefix obj/utils/, ApplyGenome.o Genome.o XMLConverter.o \
Random.o ExpressionParser.o DataFile.o )
	$(CXX) -o $@ $^ $(LIBS)

INSTALL_BIN ?= /usr/local/bin

install: all
	install $(BINARIES) $(INSTALL_BIN)
	install scripts/gait_morph.py $(INSTALL_BIN)
	install scripts/kinematic_match.py $(INSTALL_BIN)
	install scripts/ExtractValues.py $(INSTALL_BIN)
	install scripts/SmartTextMerge.py $(INSTALL_BIN)
