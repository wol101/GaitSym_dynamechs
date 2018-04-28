
CXX = mpicc
CXXFLAGS = -O4

all: run_sh 

run_sh: run_sh.o
	$(CXX) $(CXXFLAGS) $(LDLIBS) -o $@ $^
        
%.o: %.c
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $^ 

clean:
	rm -f *~ *.bak *.bck *.tmp *.o

