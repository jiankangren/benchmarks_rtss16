# This use the default g++ 5.1.0. Unfortunately, cilkview does not
# work with g++ 5.1.0
CC = /usr/local/gcc5/bin/g++

# In order to run cilkview for the program, it should be compiled with g++ 4.8
CC_CV = /export/shakespeare/home/li.jing/cilkplus/cilkplus-install/bin/g++
IPATH = -isystem /export/shakespeare/home/li.jing/cilkplus/cilkplus-install/include/c++/4.8.1
LPATH=-L/export/shakespeare/home/li.jing/cilkplus/cilkplus-install/lib64

CLANG = /usr/local/llvm-cilk/bin/clang
CLANGXX = /usr/local/llvm-cilk/bin/clang++


CILK=lu_cilk
OMP=lu_omp

CILK_CV=lu_cilk_cv

CILK_PAR=lu_cilk_par
OMP_PAR=lu_omp_par

LAUNCH=proc_launch

# Version that works with Cilkprof (compiled with LLVM)
CILK_CP=lu_cilk_cp

all:regular cilkview par

regular: $(CILK) $(OMP)
cilkview: $(CILK_CV)
par: $(CILK_PAR) $(OMP_PAR) $(LAUNCH)
cilkprof: $(CILK_CP)

# Make regular executable files
$(CILK): lu.c getoptions.c timespec_functions.cpp
	$(CC) -fcilkplus -D__CILKPLUS__ lu.c getoptions.c timespec_functions.cpp -o $(CILK) -lcilkrts -lrt

$(OMP): lu.c getoptions.c timespec_functions.cpp
	$(CC) -fopenmp -D__OPENMP__ lu.c getoptions.c timespec_functions.cpp -o $(OMP) -lrt


# Rules to make executables to work with Cilkview
$(CILK_CV): lu.c getoptions.c timespec_functions.cpp
	$(CC_CV) $(IPATH) -fcilkplus -D__CILKPLUS__ lu.c getoptions.c timespec_functions.cpp -o $(CILK_CV) $(LPATH) -lcilkrts -lrt

###
$(CILK_PAR): lu_par.c getoptions.c common.c timespec_functions.cpp
	$(CC) -Wall -fcilkplus -D__CILKPLUS__ lu_par.c getoptions.c common.c timespec_functions.cpp -o $(CILK_PAR) -lcilkrts -lrt

$(OMP_PAR): lu_par.c getoptions.c common.c timespec_functions.cpp
	$(CC) -Wall -fopenmp -D__OPENMP__ lu_par.c getoptions.c common.c timespec_functions.cpp -o $(OMP_PAR) -lrt


$(LAUNCH): launch.cpp common.c
	$(CC) launch.cpp common.c -o $(LAUNCH) -lpthread -lrt

# Rules to make Cilk Plus version that works with Cilkprof (compiled by LLVM)
$(CILK_CP): lu.c getoptions.c timespec_functions.cpp
	$(CLANGXX) -Wall -D__CILKPLUS__ -I/usr/local/gcc5/include/c++/5.1.0 -I/usr/local/gcc5/include/c++/5.1.0/x86_64-unknown-linux-gnu -I/export/shakespeare/home/sonndinh/Cilkprof/include -L/usr/local/gcc5/lib64 -L/export/shakespeare/home/sonndinh/Cilkprof/lib -fcilkplus -fcilktool -fcilktool-instr-c lu.c getoptions.c timespec_functions.cpp -o $(CILK_CP) -lcilkprof -lcilkrts -lrt -lpthread -ldl

# To make an executable that works with a modification of Cilkprof for GCC. 
# This binary generates the number of instruction counts, like Cilkview.
cilkprof_gcc_ins: lu.c getoptions.c timespec_functions.cpp
	/export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/install-gcc/bin/g++ -Wall -D__CILKPLUS__ -D__CILKPROF__ -fcilkplus -fcilktool -isystem /export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/install-gcc/include/c++/5.0.0 -I /export/shakespeare/home/sonndinh/Cilkprof/include/ -L/export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/install-gcc/lib64 lu.c getoptions.c timespec_functions.cpp -o lu_cilk_cp_gcc_ins /export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/Cilkprof/cp_inscount_original.o -lrt -lpthread -ldl /usr/local/lib/libpapi.a

# Work with a modification of Cilkprof for GCC. This generates actual time for the program.
cilkprof_gcc_time: lu.c getoptions.c timespec_functions.cpp
	/export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/install-gcc/bin/g++ -Wall -D__CILKPLUS__ -D__CILKPROF__ -fcilkplus -fcilktool -isystem /export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/install-gcc/include/c++/5.0.0 -I /export/shakespeare/home/sonndinh/Cilkprof/include/ -L/export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/install-gcc/lib64 lu.c getoptions.c timespec_functions.cpp -o lu_cilk_cp_gcc_time /export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/Cilkprof/cp_time_original.o -lrt -lpthread -ldl /usr/local/lib/libpapi.a

cilkprof_gcc_time2: lu.c getoptions.c timespec_functions.cpp
	/export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/install-gcc/bin/g++ -Wall -D__CILKPLUS__ -D__CILKPROF__ -fcilkplus -fcilktool -isystem /export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/install-gcc/include/c++/5.0.0 -I /export/shakespeare/home/sonndinh/Cilkprof/include/ -L/export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/install-gcc/lib64 lu.c getoptions.c timespec_functions.cpp -o lu_cilk_cp_gcc_time_burdened /export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/Cilkprof/cp_time_burdened.o -lrt -lpthread -ldl /usr/local/lib/libpapi.a

clean:
	rm $(CILK) $(OMP) $(CILK_CV) $(CILK_PAR) $(OMP_PAR) $(LAUNCH)