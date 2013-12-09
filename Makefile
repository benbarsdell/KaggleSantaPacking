
include Makefile.inc

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
INC_DIR = include

CXX_FLAGS  ?= -O3 -Wall #-g
NVCC_FLAGS ?= -O3 -Xcompiler -Wall $(CUDA_ARCH) #-g
LINK_FLAGS ?= -lgomp
INCLUDE    = -I$(SRC_DIR) -I$(THRUST_DIR)

all: $(BIN_DIR)/check_solution_omp $(BIN_DIR)/unit_tests_omp \
     $(BIN_DIR)/check_solution_cuda $(BIN_DIR)/unit_tests_cuda

$(OBJ_DIR)/SantaProblem_omp.o: $(SRC_DIR)/SantaProblem.cpp $(SRC_DIR)/SantaProblem.hpp
	$(GXX) -c -o $(OBJ_DIR)/SantaProblem_omp.o $(SRC_DIR)/SantaProblem.cpp $(CXX_FLAGS) $(INCLUDE) $(THRUST_OMP_FLAGS)
	cp $(SRC_DIR)/SantaProblem.hpp $(INC_DIR)/
$(OBJ_DIR)/SantaSolution_omp.o: $(SRC_DIR)/SantaSolution.cpp $(SRC_DIR)/SantaSolution.hpp
	$(GXX) -c -o $(OBJ_DIR)/SantaSolution_omp.o $(SRC_DIR)/SantaSolution.cpp $(CXX_FLAGS) $(INCLUDE) $(THRUST_OMP_FLAGS)
	cp $(SRC_DIR)/SantaSolution.hpp $(INC_DIR)/
$(OBJ_DIR)/check_solution_omp.o: $(SRC_DIR)/check_solution.cpp
	$(GXX) -c -o $(OBJ_DIR)/check_solution_omp.o $(SRC_DIR)/check_solution.cpp $(CXX_FLAGS) $(INCLUDE) $(THRUST_OMP_FLAGS)
$(BIN_DIR)/check_solution_omp: $(OBJ_DIR)/check_solution_omp.o $(OBJ_DIR)/SantaProblem_omp.o $(OBJ_DIR)/SantaSolution_omp.o
	$(GXX) -o $(BIN_DIR)/check_solution_omp $(OBJ_DIR)/check_solution_omp.o $(OBJ_DIR)/SantaProblem_omp.o $(OBJ_DIR)/SantaSolution_omp.o $(LINK_FLAGS)
$(OBJ_DIR)/unit_tests_omp.o: $(SRC_DIR)/unit_tests.cpp
	$(GXX) -c -o $(OBJ_DIR)/unit_tests_omp.o $(SRC_DIR)/unit_tests.cpp $(CXX_FLAGS) $(INCLUDE) $(THRUST_OMP_FLAGS)
$(BIN_DIR)/unit_tests_omp: $(OBJ_DIR)/unit_tests_omp.o $(OBJ_DIR)/SantaProblem_omp.o $(OBJ_DIR)/SantaSolution_omp.o
	$(GXX) -o $(BIN_DIR)/unit_tests_omp $(OBJ_DIR)/unit_tests_omp.o $(OBJ_DIR)/SantaProblem_omp.o $(OBJ_DIR)/SantaSolution_omp.o $(LINK_FLAGS)

$(OBJ_DIR)/SantaProblem_cuda.o: $(SRC_DIR)/SantaProblem.cpp $(SRC_DIR)/SantaProblem.hpp
	cp $(SRC_DIR)/SantaProblem.cpp $(SRC_DIR)/SantaProblem.cu
	$(NVCC) -c -o $(OBJ_DIR)/SantaProblem_cuda.o $(SRC_DIR)/SantaProblem.cu $($(NVCC)_FLAGS) $(INCLUDE) $(THRUST_CUDA_FLAGS)
	rm $(SRC_DIR)/SantaProblem.cu
	cp $(SRC_DIR)/SantaProblem.hpp $(INC_DIR)/
$(OBJ_DIR)/SantaSolution_cuda.o: $(SRC_DIR)/SantaSolution.cpp $(SRC_DIR)/SantaSolution.hpp
	cp $(SRC_DIR)/SantaSolution.cpp $(SRC_DIR)/SantaSolution.cu
	$(NVCC) -c -o $(OBJ_DIR)/SantaSolution_cuda.o $(SRC_DIR)/SantaSolution.cu $($(NVCC)_FLAGS) $(INCLUDE) $(THRUST_CUDA_FLAGS)
	rm $(SRC_DIR)/SantaSolution.cu
	cp $(SRC_DIR)/SantaSolution.hpp $(INC_DIR)/
$(OBJ_DIR)/check_solution_cuda.o: $(SRC_DIR)/check_solution.cpp
	cp $(SRC_DIR)/check_solution.cpp $(SRC_DIR)/check_solution.cu
	$(NVCC) -c -o $(OBJ_DIR)/check_solution_cuda.o $(SRC_DIR)/check_solution.cu $($(NVCC)_FLAGS) $(INCLUDE) $(THRUST_CUDA_FLAGS)
	rm $(SRC_DIR)/check_solution.cu
$(BIN_DIR)/check_solution_cuda: $(OBJ_DIR)/check_solution_cuda.o $(OBJ_DIR)/SantaProblem_cuda.o $(OBJ_DIR)/SantaSolution_cuda.o
	$(NVCC) -o $(BIN_DIR)/check_solution_cuda $(OBJ_DIR)/check_solution_cuda.o $(OBJ_DIR)/SantaProblem_cuda.o $(OBJ_DIR)/SantaSolution_cuda.o $(LINK_FLAGS)
$(OBJ_DIR)/unit_tests_cuda.o: $(SRC_DIR)/unit_tests.cpp
	cp $(SRC_DIR)/unit_tests.cpp $(SRC_DIR)/unit_tests.cu
	$(NVCC) -c -o $(OBJ_DIR)/unit_tests_cuda.o $(SRC_DIR)/unit_tests.cu $($(NVCC)_FLAGS) $(INCLUDE) $(THRUST_CUDA_FLAGS)
	rm $(SRC_DIR)/unit_tests.cu
$(BIN_DIR)/unit_tests_cuda: $(OBJ_DIR)/unit_tests_cuda.o $(OBJ_DIR)/SantaProblem_cuda.o $(OBJ_DIR)/SantaSolution_cuda.o
	$(NVCC) -o $(BIN_DIR)/unit_tests_cuda $(OBJ_DIR)/unit_tests_cuda.o $(OBJ_DIR)/SantaProblem_cuda.o $(OBJ_DIR)/SantaSolution_cuda.o $(LINK_FLAGS)

test: $(BIN_DIR)/unit_tests_omp
	OMP_NUM_THREADS=1 $(BIN_DIR)/unit_tests_omp

clean:
	rm -f $(BIN_DIR)/* $(OBJ_DIR)/*.o $(INC_DIR)/*.h $(INC_DIR)/*.hpp
