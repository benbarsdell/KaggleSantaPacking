
CUDA_ARCH  ?= -arch=sm_21
#CUDA_ARCH ?= -gencode arch=compute_20,code=sm_21 \
#             -gencode arch=compute_30,code=sm_30 \
#             -gencode arch=compute_35,code=sm_35
CUDA_DIR   ?= /usr/local/cuda
THRUST_DIR ?= $(CUDA_DIR)/include

GXX  = g++
NVCC = nvcc

THRUST_OMP_FLAGS ?= -fopenmp -DTHRUST_DEVICE_BACKEND=THRUST_DEVICE_BACKEND_OMP -lgomp
#THRUST_OMP_FLAGS ?= -fopenmp -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_OMP -lgomp
