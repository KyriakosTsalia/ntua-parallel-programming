/*
 *  dmv_gpu.cu -- Template for DMV GPU kernels
 *
 *  Copyright (C) 2010-2013, Computing Systems Laboratory (CSLab)
 *  Copyright (C) 2010-2013, Vasileios Karakasis
 */ 
#include <stdio.h>
#include "dmv.h"

/*
 *  Utility function to get the thread ID within the
 *  global working space.
 */ 
__device__ int get_global_tid()
{
    return (gridDim.x*blockIdx.y + blockIdx.x)*blockDim.x*blockDim.y +
        blockDim.x*threadIdx.y + threadIdx.x;
}

/*
 *  Utility function to get the thread ID within the
 *  local/block working space.
 */ 
__device__ int get_local_tid()
{
    return blockDim.x*threadIdx.y + threadIdx.x;
}

/*
 *  Naive kernel
 */ 
__global__ void dmv_gpu_naive(const value_t *a, const value_t *x, value_t *y,
                              size_t n)
{
    /*
     * FILLME: fill the code for the naive kernel.
     */ 
	 size_t tid,j;
	 value_t yi;
	 
	 for (tid=get_global_tid();tid<n;tid+=blockDim.x*gridDim.x)
		{
		yi = 0;
		for (j = 0; j < n; ++j) {
            yi += a[tid*n+j]*x[j];
        }

        y[tid] = yi;
		}
}

/*
 *  Coalesced memory acceses
 */
__global__ void dmv_gpu_coalesced(const value_t *a, const value_t *x,
                                  value_t *y, size_t n)
{
    /*
     * FILLME: fill the code for the coalesced kernel.
     */ 
	 size_t tid,j;
	 value_t yi;
	 
	 for (tid=get_global_tid();tid<n;tid+=blockDim.x*gridDim.x)
		{
		yi = 0;
		for (j = 0; j < n; ++j) {
            yi += a[j*n+tid]*x[j];
        }

        y[tid] = yi;
		}
}

/*
 *  Use of shared memory
 */
__global__ void dmv_gpu_shmem(const value_t *a, const value_t *x, value_t *y,
                              size_t n)
{
    /*
     * FILLME: fill the code for the shared memory kernel.
     */ 
	extern __shared__ value_t x_shared[]; 
	size_t tid,i,j;
	value_t yi;
	
	for (i=get_local_tid();(i<n && i<48*1024/sizeof(value_t));i+=blockDim.x)		x_shared[i]=x[i];
	__syncthreads();
	
	for (tid=get_global_tid();tid<n;tid+=blockDim.x*gridDim.x)
		{
		yi = 0;
		for (j = 0; j < n; ++j) {
			if (j>=48*1024/sizeof(value_t))             yi += a[j*n+tid]*x[j];
			else	yi += a[j*n+tid]*x_shared[j];
        }

        y[tid] = yi;
		}
}
