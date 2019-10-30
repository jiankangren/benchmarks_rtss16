# This use the default g++ 5.1.0. Unfortunately, cilkview does not
# work with g++ 5.1.0
CC=/usr/local/gcc5/bin/g++

# In order to run cilkview for the program, it should be compiled with g++ 4.8
CC_CV = /export/shakespeare/home/li.jing/cilkplus/cilkplus-install/bin/g++
IPATH = -isystem /export/shakespeare/home/li.jing/cilkplus/cilkplus-install/include/c++/4.8.1 \
		-I /export/shakespeare/home/sonndinh/cilktools-linux-004421/include/cilktools
LPATH = -L/export/shakespeare/home/li.jing/cilkplus/cilkplus-install/lib64 #\
#		-L/export/shakespeare/home/sonndinh/cilktools-linux-004421/lib64

CLANG = /usr/local/llvm-cilk/bin/clang
CLANGXX = /usr/local/llvm-cilk/bin/clang++


# Regular executables
CILK=ch_cilk
OMP=ch_omp
CILK_LLVM = ch_cilk_llvm

# Version that can be measured with Cilkview
CILK_CV=ch_cilk_cv

# Executables that can be run multiple instances simultaneously
CILK_PAR=ch_cilk_par
OMP_PAR=ch_omp_par

# Helper executable to lauch processes
LAUNCH=proc_launch

# Version that works with Cilkprof (compiled with LLVM)
CILK_CP=ch_cilk_cp

all: regular cilkview par

regular: $(CILK) $(OMP)
cilkview: $(CILK_CV)
par: $(CILK_PAR) $(OMP_PAR) $(LAUNCH)
cilkprof: $(CILK_CP)
llvm: $(CILK_LLVM)

# Rules to make regular Cilk Plus and OMP version with GCC
${CILK}: cholesky.c getoptions.c rand.c timespec_functions.cpp
	$(CC) -Wall -fcilkplus -D__CILKPLUS__ cholesky.c getoptions.c rand.c timespec_functions.cpp -o ${CILK} -lcilkrts -lrt

${OMP}: cholesky.c getoptions.c rand.c timespec_functions.cpp
	$(CC) -Wall -g -fopenmp -D__OPENMP__ cholesky.c getoptions.c rand.c timespec_functions.cpp -o ${OMP} -lrt


# Rules to make executables to work with Cilkview
$(CILK_CV): cholesky.c getoptions.c rand.c timespec_functions.cpp
	$(CC_CV) -fcilkplus -D__CILKPLUS__ -D__CILKVIEW__ $(IPATH) $(LPATH) cholesky.c getoptions.c rand.c timespec_functions.cpp -o $(CILK_CV) -lcilkrts -lrt #-lxed -lpindwarf -l:cilkview-tool.so


###
$(CILK_PAR): cholesky_par.c getoptions.c common.c rand.c timespec_functions.cpp
	$(CC) -Wall -fcilkplus -D__CILKPLUS__ cholesky_par.c getoptions.c common.c rand.c timespec_functions.cpp -o $(CILK_PAR) -lcilkrts -lrt

$(OMP_PAR): cholesky_par.c getoptions.c common.c rand.c timespec_functions.cpp
	$(CC) -Wall -fopenmp -D__OPENMP__ cholesky_par.c getoptions.c common.c rand.c timespec_functions.cpp -o $(OMP_PAR) -lrt


$(LAUNCH): launch.cpp common.c
	$(CC) launch.cpp common.c -o proc_launch -lpthread -lrt

# 
# Rules to make Cilk Plus version that works with Cilkprof (compiled by LLVM)
$(CILK_CP): cholesky.c getoptions.c rand.c timespec_functions.cpp
	$(CLANGXX) -Wall -D__CILKPLUS__ -I/usr/local/gcc5/include/c++/5.1.0 -I/usr/local/gcc5/include/c++/5.1.0/x86_64-unknown-linux-gnu -I/export/shakespeare/home/sonndinh/Cilkprof/include -L/usr/local/gcc5/lib64 -L/export/shakespeare/home/sonndinh/Cilkprof/lib -fcilkplus -fcilktool -fcilktool-instr-c cholesky.c getoptions.c rand.c timespec_functions.cpp -o $(CILK_CP) -lcilkprof -lcilkrts -lrt -lpthread -ldl 

$(CILK_LLVM): cholesky.c getoptions.c rand.c timespec_functions.cpp
	$(CLANGXX) -Wall -D__CILKPLUS__ -I/usr/local/gcc5/include/c++/5.1.0 -I/usr/local/gcc5/include/c++/5.1.0/x86_64-unknown-linux-gnu -I/export/shakespeare/home/sonndinh/Cilkprof/include -L/usr/local/gcc5/lib64 -L/export/shakespeare/home/sonndinh/Cilkprof/lib -fcilkplus cholesky.c getoptions.c rand.c timespec_functions.cpp -o $(CILK_LLVM) -lcilkrts -lrt -lpthread -ldl

# To make an executable that works with a modification of Cilkprof for GCC. 
# This binary generates the number of instruction counts, like Cilkview. 
cilkprof_gcc_ins: cholesky.c getoptions.c rand.c timespec_functions.cpp
	/export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/install-gcc/bin/g++ -Wall -D__CILKPLUS__ -D__CILKPROF__ -fcilkplus -fcilktool -isystem /export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/install-gcc/include/c++/5.0.0 -I /export/shakespeare/home/sonndinh/Cilkprof/include/ -L/export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/install-gcc/lib64 cholesky.c getoptions.c rand.c timespec_functions.cpp -o ch_cilk_cp_gcc_ins /export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/Cilkprof/cp_inscount_original.o -lrt -lpthread -ldl /usr/local/lib/libpapi.a

# Work with a modification of Cilkprof for GCC. This generates actual time for the program.
cilkprof_gcc_time: cholesky.c getoptions.c rand.c timespec_functions.cpp
	/export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/install-gcc/bin/g++ -Wall -D__CILKPLUS__ -D__CILKPROF__ -fcilkplus -fcilktool -isystem /export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/install-gcc/include/c++/5.0.0 -I /export/shakespeare/home/sonndinh/Cilkprof/include/ -L/export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/install-gcc/lib64 cholesky.c getoptions.c rand.c timespec_functions.cpp -o ch_cilk_cp_gcc_time /export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/Cilkprof/cp_time_original.o -lrt -lpthread -ldl /usr/local/lib/libpapi.a

cilkprof_gcc_time2: cholesky.c getoptions.c rand.c timespec_functions.cpp
	/export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/install-gcc/bin/g++ -Wall -D__CILKPLUS__ -D__CILKPROF__ -fcilkplus -fcilktool -isystem /export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/install-gcc/include/c++/5.0.0 -I /export/shakespeare/home/sonndinh/Cilkprof/include/ -L/export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/install-gcc/lib64 cholesky.c getoptions.c rand.c timespec_functions.cpp -o ch_cilk_cp_gcc_time_burdened /export/shakespeare/home/li.jing/cilktools/gcc_cilkprof/Cilkprof/cp_time_burdened.o -lrt -lpthread -ldl /usr/local/lib/libpapi.a


clean:
	rm $(CILK) $(OMP) $(CILK_CV) $(CILK_PAR) $(OMP_PAR) $(LAUNCH)