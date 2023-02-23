---
title: 6. CUDA Programming
date: 2022-07-21
category: hipc
layout: post
---

# Overview

In this practical, we will learn heterogeneous programming with a GPU accelerator using CUDA.

We'll start with some basic CUDA programs, and gradually move towards more advanced features and functions of CUDA. Through this lab, you will learn how to run CUDA on a local machine as well as on Viking.

**Note:** You will find the official [NVIDIA CUDA Programming Guide](https://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html#abstract) extremely handy during this lab.

# Prepration

**Check the Driver**

Before we start, we need to first check the information of the NVIDIA graphics driver and CUDA. In a new terminal:

```
$ nvidia-smi
```

An example output (note the output is just an example; your machine could print something different depending on the hardware and the version of the graphics driver):

```
+-----------------------------------------------------------------------------+
| NVIDIA-SMI 470.103.01   Driver Version: 470.103.01   CUDA Version: 11.4     |
|-------------------------------+----------------------+----------------------+
| GPU  Name        Persistence-M| Bus-Id        Disp.A | Volatile Uncorr. ECC |
| Fan  Temp  Perf  Pwr:Usage/Cap|         Memory-Usage | GPU-Util  Compute M. |
|                               |                      |               MIG M. |
|===============================+======================+======================|
|   0  NVIDIA GeForce ...  On   | 00000000:01:00.0  On |                  N/A |
|  5%   53C    P0    40W / 180W |    578MiB /  8116MiB |      0%      Default |
|                               |                      |                  N/A |
+-------------------------------+----------------------+----------------------+

+-----------------------------------------------------------------------------+
| Processes:                                                                  |
|  GPU   GI   CI        PID   Type   Process name                  GPU Memory |
|        ID   ID                                                   Usage      |
|=============================================================================|
|    0   N/A  N/A      1435      G   /usr/lib/xorg/Xorg                315MiB |
|    0   N/A  N/A      1781      G   /usr/bin/gnome-shell               54MiB |
|    0   N/A  N/A      2633      G   ...urePrivateNetworkRequests      155MiB |
|    0   N/A  N/A      5869      G   ...oken=13306174281309787638       47MiB |
+-----------------------------------------------------------------------------+
```

The GPU we have on the managed PCs in the lab is an _NVIDIA Quadro P1000_, which has 640 CUDA Cores (5 SMs and each has 128 cores).

**Check the CUDA Compiler**

In a terminal, check the `nvcc` compiler with:

```
$ nvcc --version
```

An example output (note the output is just an example; the University machine may print something different):

```
nvcc: NVIDIA (R) Cuda compiler driver
Copyright (c) 2005-2020 NVIDIA Corporation
Built on Thu_Jun_11_22:26:38_PDT_2020
Cuda compilation tools, release 11.0, V11.0.194
Build cuda_11.0_bu.TC445_37.28540450_0
```

If it says the package needs installing, give it 10 minutes, log out and log back in again. Report to the instructors if there are still problems with the environment after 10 minutes.

> **Note for mac users:** If you are running macOS, then it is very unlikely you have CUDA installed on your machine, and with a CUDA-compatible GPU in your Mac. Since CUDA 11.0, macOS is not a supported environment for CUDA. The last supported macOS version would be 10.13.x (based on 10.2). If you have a CUDA-compatible GPU in your Mac, and you would like to continue, you are encouraged to try but we can provide no support on this. Here are [some instructions for installing CUDA on Mac](https://developer.download.nvidia.com/compute/cuda/6_5/rel/docs/CUDA_Getting_Started_Mac.pdf). Alternatively, you can use a managed PC or Viking. <br/><br/>
{: .block-warning }


> # Exercise 1
>
> Re-write the "hello, world" example from Unit 7. Change the `<<M , T>>` from `<<1, 1>>` to `<<3, 3>>`. Within each thread, print its own block ID and thread ID. <br/><br/>  
{: .block-danger }

> # Exercise 2
>
> **Part 1** Practice the _vect_add_ example in Unit 7. Compare the performance of different Kernel configurations (note your code has to be adjusted according to the kernel configuration!) and report your results. Produce a diagram/table that shows how performance changes against the Kernel configuration.
>
> **Part 2** One issue of _vect_add_ is that it is not friendly when _N_ is not multiples of `blockDim.x`. Adjust the code so that it can handle arbitrary vector sizes (N). <br/><br/>
{: .block-danger }

> # Exercise 3
>
> **Part 1:** Vectorise the Monte Carlo Pi exercise we used in the previous practicals with CUDA.  
>
> You may want to start from the following single-thread CPU version:
> 
> ```c
> #include <stdio.h>
> #include <stdlib.h>
> #include <math.h>
> #include <time.h>
> 
> int main(int argc, char* argv[]) {
>     long niter = 1000000L;
>     double x,y;
>     int i;
>     int count = 0;
>     double z;
>     double pi;
>     struct timespec tstart={0,0}, tend={0,0};
>     srand(time(NULL));
> 
>     clock_gettime(CLOCK_MONOTONIC, &tstart);
> 
>     //main loop
>     for (i = 0; i < niter; ++i) {
>         //get random points
>         x = (double)random() / RAND_MAX;
>         y = (double)random() / RAND_MAX;
>         z = sqrt((x*x)+(y*y));
>         //check to see if point is in unit circle
>         if (z <= 1) {
>             ++count;
>         }
>     }
>     //p = 4(m/n)
>     pi = ((double)count/(double)niter)*4.0;
> 
>     clock_gettime(CLOCK_MONOTONIC, &tend);
> 
>     printf("Pi: %f\n", pi);
>     printf("Time taken %.6f ms\n",
>            ((double)tend.tv_sec * 0.001 + 1.0e-6*tend.tv_nsec) -
>            ((double)tstart.tv_sec * 0.001 + 1.0e-6*tstart.tv_nsec));
> 
>     return 0;
> }
> ```
> 
> Note that CUDA has its own built-in random number generators. Try to figure out how to use them. You may find [this page](https://docs.nvidia.com/cuda/curand/host-api-overview.html) useful.
>
> **Part 2:** Just like what we did for the OpenMP lab -- Try to understand how to implement a _reduction_ in CUDA and apply the technique to your code.
>
> **Part 3:** Could you find out the best Kernel configuration that produces the highest performance?  <br/><br/>
{: .block-danger }


> # Exercise 4
>
> Below is an experiment that uses an increment kernel shown in the following code, with strided accesses to the input array. Compile and run the program and check what the result looks like. Try to understand what is causing this.
> 
> 
> ```c
> #include <stdio.h>
> #include <assert.h>
> #include <cuda.h>
> #include <cuda_runtime.h>
> 
> // Convenience function for checking CUDA runtime API results
> // can be wrapped around any runtime API call. No-op in release builds.
> inline cudaError_t checkCuda(cudaError_t result) {
> #if defined(DEBUG) || defined(_DEBUG)
>     if (result != cudaSuccess) {
>         fprintf(stderr, "CUDA Runtime Error: %s\n", cudaGetErrorString(result));
>         assert(result == cudaSuccess);
>     }
> #endif
>     return result;
> }
> 
> __global__ void stride(double* a, int s) {
>     int i = (blockDim.x * blockIdx.x + threadIdx.x) * s;
>     a[i] = a[i] + 1;
> }
> 
> void runTest(int deviceId, int nMB) {
>     int blockSize = 256;
>     float ms;
> 
>     double *d_a;
>     cudaEvent_t startEvent, stopEvent;
> 
>     int n = nMB*1024*1024/sizeof(double);
> 
>     // NB:  d_a(33*nMB) for stride case
>     checkCuda( cudaMalloc(&d_a, n * 33 * sizeof(double)) );
> 
>     checkCuda( cudaEventCreate(&startEvent) );
>     checkCuda( cudaEventCreate(&stopEvent) );
> 
>     printf("Stride, Bandwidth (GB/s):\n");
> 
>     stride<<<n/blockSize, blockSize>>>(d_a, 1); // warm up
> 
>     for (int i = 1; i <= 32; i++) {
>         checkCuda( cudaMemset(d_a, 0, n * sizeof(double)) );
> 
>         checkCuda( cudaEventRecord(startEvent,0) );
>         stride<<<n/blockSize, blockSize>>>(d_a, i);
>         checkCuda( cudaEventRecord(stopEvent,0) );
>         checkCuda( cudaEventSynchronize(stopEvent) );
> 
>         checkCuda( cudaEventElapsedTime(&ms, startEvent, stopEvent) );
>         printf("%d\t%f\n", i, 2*nMB/ms);
>     }
> 
>     checkCuda( cudaEventDestroy(startEvent) );
>     checkCuda( cudaEventDestroy(stopEvent) );
>     cudaFree(d_a);
> }
> 
> int main(int argc, char **argv) {
>     int nMB = 4;
>     int deviceId = 0;
> 
>     cudaDeviceProp prop;
> 
>     checkCuda( cudaSetDevice(deviceId) );
>     checkCuda( cudaGetDeviceProperties(&prop, deviceId) );
>     printf("Device: %s\n", prop.name);
>     printf("Transfer size (MB): %d\n", nMB);
> 
>     runTest(deviceId, nMB);
> }
> ```
> <br/>
{: .block-danger }

### Memory Coalescing

One terminology that you won't hear very often in CPU programming is memory coalescing. In CUDA, you typically have 32 threads (a warp) working together in unison. So often, you have 32 threads, each wanting to write a 32-bit result to the main GPU memory. In order to accomplish this task with an absolute minimum number of memory transactions, it is best if the memory accesses are coalesced such that there are no bank conflicts.

More details can be found in the [CUDA Programming Guide: Section 5](https://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html#device-memory-accesses).

> # Exercise 5
>
> In this final exercise, we will try to run our CUDA programs on Viking.
> 
> In order to run CUDA programs, you need to specify the need for GPU support so Viking will assign you a node with a GPU enabled. To do so, in your job script, add/update:
> 
> ```bash
> #SBATCH --partition=gpu
> #SBATCH --gres=gpu:1
> ```
> 
> You also need to load the CUDA module with `module load` in order to compile your applications, and in order to run your applications on an assigned node.
> 
> Below is an example, adapted from the [Viking documentation](https://wiki.york.ac.uk/display/RCS/VK4%29+Job+script+configuration#VK4)Jobscriptconfiguration-GPUjobs):
> 
> ```bash
> #!/bin/bash
> #SBATCH --job-name=cuda_job                    # Job name
> #SBATCH --ntasks=1                             # Run a single task...
> #SBATCH --cpus-per-task=1                      # ...with a single CPU
> #SBATCH --mem=1gb                              # Job memory request
> #SBATCH --time=01:00:00                        # Time limit hrs:min:sec
> #SBATCH --output=cuda_job_%j.log               # Standard output and error log
> #SBATCH --partition=gpu                        # Select the GPU nodes...
> #SBATCH --gres=gpu:1                           # ...and a single GPU
> 
> module load system/CUDA/11.0.2-GCC-9.3.0
> 
> echo `date`: executing gpu_test on host $HOSTNAME with $SLURM_CPUS_ON_NODE cpu cores
> echo
> cudaDevs=$(echo $CUDA_VISIBLE_DEVICES | sed -e 's/,/ /g')
> echo I can see GPU devices $CUDA_VISIBLE_DEVICES
> echo
> 
> /users/{YOUR_USER_NAME}/scratch/{YOUR_CODE_FILENAME}
> ```
> <br/>
{: .block-danger }