# This use the default g++ 5.1.0. Unfortunately, cilkview does not
# work with g++ 5.1.0
CC=g++

# In order to run cilkview for the program, it should be compiled with g++ 4.8
CC_CV=/export/shakespeare/home/li.jing/cilkplus/cilkplus-install/bin/g++
IPATH=-I/export/shakespeare/home/li.jing/cilkplus/cilkplus-install/include/cilk
LPATH=-L/export/shakespeare/home/li.jing/cilkplus/cilkplus-install/lib \
		-L/export/shakespeare/home/li.jing/cilkplus/cilkplus-install/lib64


CILK=fft_cilk
OMP=fft_omp

CILK_CV=fft_cilk_cv

CILK_PAR=fft_cilk_par
OMP_PAR=fft_omp_par

LAUNCH=proc_launch

all: regular cilkview par

regular: $(CILK) $(OMP)
cilkview: $(CILK_CV)
par: $(CILK_PAR) $(OMP_PAR) $(LAUNCH)

${CILK}: fft.c getoptions.c timespec_functions.cpp
	$(CC) -Wall -fcilkplus -D__CILKPLUS__ fft.c getoptions.c timespec_functions.cpp -o ${CILK} -lcilkrts -lrt

${OMP}: fft.c getoptions.c timespec_functions.cpp
	$(CC) -Wall -g -fopenmp -D__OPENMP__ fft.c getoptions.c timespec_functions.cpp -o ${OMP} -lrt


# Rules to make executables to work with Cilkview
$(CILK_CV): fft.c getoptions.c timespec_functions.cpp
	$(CC_CV) $(IPATH) -fcilkplus -D__CILKPLUS__ fft.c getoptions.c timespec_functions.cpp -o $(CILK_CV) $(LPATH) -lcilkrts -lrt


###
$(CILK_PAR): fft_par.c getoptions.c common.c timespec_functions.cpp
	$(CC) -Wall -fcilkplus -D__CILKPLUS__ fft_par.c getoptions.c common.c timespec_functions.cpp -o $(CILK_PAR) -lcilkrts -lrt

$(OMP_PAR): fft_par.c getoptions.c common.c timespec_functions.cpp
	$(CC) -Wall -fopenmp -D__OPENMP__ fft_par.c getoptions.c common.c timespec_functions.cpp -o $(OMP_PAR) -lrt


$(LAUNCH): launch.cpp common.c
	$(CC) launch.cpp common.c -o proc_launch -lpthread -lrt

clean:
	rm $(CILK) $(OMP) $(CILK_CV) $(CILK_PAR) $(OMP_PAR) $(LAUNCH)