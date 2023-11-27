---
title: 9. Advanced Accelerators
date: 2022-07-27
category: hipc
layout: post
---

# Overview

In the last lecture, we have introduced the basis of GPU hardware and CUDA programming. We only touched the surface of CUDA and it is far from reaching its full potential. In this lecture, we will look at some more advanced topics in CUDA, including performance considerations, and more scheduling and memory. Before we start, make sure you have understand everything in the last lecture as otherwise it would be difficult to proceed.

The following topics will be covered in this lecture:

- Advanced Scheduling
- Advanced Execution
- Advanced Memory Models
- Performance Profiling, Tuning and Optimization

* * *

# Advanced Scheduling

## Introduction

Before we start, recap what we have learnt at the end of the last lecture on thread block organization. The kernel configuration can be 2-dimensional (as well as 3-dimensional). This is useful for algorithms that are with 2d & 3d data layouts. To access the second (and the third) dimension, use the `.y` attribute (and `.z` for indexing the third dimension) with a 3-vector (`dim3`).

To illustrate this layout:

![3D Kernel Indexing](../../assets/unit-9/kernel-indexing.png)

_**Figure 1:** 3D Kernel Indexing_
{: style="color:gray; font-size: 90%; text-align: center;" }

The thread-block-grid hierarchy forms the basic concept of CUDA execution model from the software perspective. As explained earlier, this software abstraction facilities GPU programming in a way that the programmer does not need to know every detail of the hardware. This also improves the compatibility as otherwise one program written for one GPU would not work for another GPU, due to the difference in their hardware specifications.

![Execution Model of CUDA](../../assets/unit-9/execution-model.png)

_**Figure 2:** Execution Model of CUDA_  
{: style="color:gray; font-size: 90%; text-align: center;" }

Mapping from the software abstraction into the hardware is done by CUDA, with a process so called `scheduling`. Scheduling decides when and how to execute which thread on which SM/CUDA core.

## Warp Scheduling

In CUDA, threads are scheduled in a group of 32 (warp) to improve the efficiency and reduce the scheduling and instruction dispatching overhead. A wrap (sometimes known as a sub-group) is thus a set of 32 threads, within a thread block, and all that threads in the warp execute the same instruction on an Stream Multiprocessor (SM).

Organising threads into warps have the following major benefits:

- reducing hardware manufacturing cost
- lower runtime operation electricity cost
- enable coalescing of memory accesses (which will be the topic of some later post)

Once a thread block is launched on a multiprocessor (SM), all of its warps are resident until their execution finishes. A new block will not be launched on an SM util there is enough free shared memory and sufficient number of free registers for all warps.

Once the currently executing warp stalls, for example, waiting for a memory request, waiting to run on a particular under-pressure functional unit etc, the scheduler pends this warp and runs the next one available that’s ready to run. This process is known as a __context switching__, which transfers control from one warp to another warp. The data from the previous warp remains in the register file and can be quickly resumed when its operands become ready later. As all of a warp's register value and PC are stored in the register file, and the shared memory (and cache) remain in place since these are shared between all the warps in the thread block, the context switching is much more light-weighted than a traditional CPU context switching.

If more than one warps are eligible for execution, the parent SM uses a warp scheduling policy for deciding which warp gets the next fetched instruction. There is a number of different scheduling algorithms that are eligible for execution:

- Round Robin (RR) - Instructions are fetched in round robin manner. RR makes sure that SMs are kept busy and no clock cycles are wasted on memory latencies.
- Least Recently Fetched (LRF) - In this policy, warp for which instruction has not been fetched for the longest time gets priority in the fetching of an instruction.
- Fair (FAIR) - In this policy, the scheduler makes sure all warps are given ‘fair’ opportunity in the number of instruction fetched for them. It fetches instruction to a warp for which minimum number of instructions have been fetched.
- Thread block-based CAWS (criticality aware warp scheduling) - The emphasis of this scheduling policy is on improving the execution time of the thread blocks. It allocated more time resources to the warp that shall take the longest time to execute. By giving priority to the most critical warp, this policy allows thread blocks to finish faster, such that the resources become available quicker.

> **Note**
>
> Warps can be executed by the SMs in any order, thus assumptions on which one runs first should be avoided when programming.
{: .block-warning }

In order to take advantage of the warp architecture, we need to understand _(a) how to manage control flow divergence_ (next section) and _(b) how to coalesce memory accesses_ (will be discussed in advanced memory). If each thread in a warp takes a different execution path or if each thread accesses significantly divergent memory then the benefits of the warp architecture are lost and performance will significantly degradation.

## Block Partitioning

We now know a warp is formulated of 32 threads but how does it pick the threads? Generally it is based on the thread index (each warp consists of 32 threads of consecutive thredIdx values). There is a few complication based on the dimension of the thread block, which are discussed below:

1. For 1D thread block:
	- only threadIdx.x is used, threadIdx.x values within a warp are consecutive and increasing.
	- For a warp size of 32:
		- warp 0: thread 0 ~ thread 31
		- warp 1: thread 32 ~ thread 63
		- warp n: thread 32 × n ~ thread 32(n + 1) - 1
	- For a block of which the size is not a multiple of 32:
		- the last warp will be padded with extra threads to fill up the 32 threads.
2. For 2D thread block:
	- the dimensions will be projected into a linear order before partitioning into warps
	- determine the linear order: place the rows with larger y and z coordinates after those with lower ones.
3. For 3D thread block:
	- first place all threads of which the threadIdx.z value is 0 into the linear order. Among these threads, they are treated as a 2D block.
	- Example: a 3D thread block of dimensions 2 × 8 × 4 (total 64 threads):
		- warp 0: T(0,0,0) ~ T(0,7,3)
		- warp 1: T(1,0,0) ~ T(1,7,3)

## Dealing with Divergence

Conditional branch instructions cause threads to diverge. However, the implementation of GPU allows only one PC (program counter) at a time inside a warp. An obvious question would then be: what happened if there is divergence inside a warp?

Assuming we have something in a kernel function as:

```c
if (some_condition):
    do_stuff_A();
else:
    do_stuff_B()
```

The `if` condition would cause divengency based on if `some_condition` is satisfied or not. Say if `some_condition` is if the working thread is with odd-number, then half of the threads will run `do_stuff_A()` and the other half are masked (this means consuming resources without actually running anything), after that is finished, the warp starts to execute the next instruction, in which case the even-number will execute `do_stuff_B()`, and the odd-number threads would be masked. This is illustrated as follows:

![Branch Divergence](../../assets/unit-9/branch-divergence.png)

_**Figure 3:** Branch Divergence_  
{: style="color:gray; font-size: 90%; text-align: center;" }

You may already notice this is not particular efficient as the total execution time is then `total_time = t(Path_A) + t(Path_B)`, instead of the longest time `max(t(Path_A), t(Path_B))`. Things can go differently, if the conditional is such that all work-items in the same warp take the same path. Assuming we can make the divergence on a warp-basis, say we do things differently based on if it is an odd warp or an even-numbered warp, then it is possible for these warps to be executed concurrently, and without waiting and masking inside a warp.


## Dynamic Parallelism

*Dynamic Parallelism* is an extension to the CUDA programming model enabling a CUDA kernel to create and synchronize with new work directly on the GPU. It allows dynamical creation of parallel threads on-the-air, based on decisions that are made at runtime based on the incoming data. This extension can take the advantage of the GPU's hardware schedulers and load balancers dynamically, as parallel thread can be generated inline within a kernel at run-time.

> **Note**
>
> Dynamical parallelism is only supported by devices with Compute Capability >= 3.5.  
{: .block-warning }

The discussion of dynamic parallelism is slightly beyond what we should deliver here due to its complexity. If you are interested in, more information can be found on the [CUDA Programming Guide](https://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html?highlight=scheduling#cuda-dynamic-parallelism).

# Advanced Execution (Concurrency)
## Asynchronous Execution
Concurrency with asynchronous execution can significantly improve the throughput as it minimize the latency by formulating a pipeline. Asynchronous execution adds the ability to perform multiple CUDA operations simultaneously, allowing overlap of data transfer and kernel execution.

![0b1cdcdcdefa5cac0e143f6b2521b9ca.png](../../assets/unit-9/concurrent-model.png)
_**Figure 4:** Concurrent vs Serial Model_  
{: style="color:gray; font-size: 90%; text-align: center;" }


Asynchronous execution in CUDA is based on a **Concurrent Model**, which is done through  `Streams`. In CUDA, streams can be defined using the API `cudaStreamCreate()`:

```
cudaStream_t stream[2];
for (int i = 0; i < 2; ++i)
    cudaStreamCreate(&stream[i]);
float* hostPtr;
cudaMallocHost(&hostPtr, 2 * size);
```

Once we have defined a stream, we can then use _asynchronous data transfers_ (requires page-locked host memory, i.e., memory allocated with `cudaMallocHost(…)`):

```
cudaStreamCreate(&stream1);
cudaStreamCreate(&stream2);

cudaMemcpyAsync(dst1, src1, size, dir, stream1);
kernel<<<grid, block, 0, stream1>>>(…);
cudaMemcpyAsync(dst2, src2, size, dir, stream2);
kernel<<<grid, block, 0, stream2>>>(…);
```

Finally, we can destrory the streams with `cudaStreamDestroy()`:

```c
for (int i = 0; i < 2; ++i)
    cudaStreamDestroy(stream[i]);
```

### Synchronization of Streams

With asynchronous execution, it is important to synchronous the threads to ensure data is properly transfered or processed, and to reduce race conditions. For CUDA streams, synchronization can be done explicitly with:

- `cudaDeviceSynchronize()` waits until all preceding commands in all streams of all host threads have completed.
- `cudaStreamSynchronize()` takes a stream as a parameter and waits until all preceding commands in the given stream have completed. It can be used to synchronize the host with a specific stream, allowing other streams to continue executing on the device.
- `cudaStreamWaitEvent()` takes a stream and an event as parameters (see Time a CUDA Program to see how to define an event) and makes all the commands added to the given stream after the call to `cudaStreamWaitEvent()` delay their execution until the given event has completed.
- `cudaStreamQuery()` provides applications with a way to know if all preceding commands in a stream have completed.

There will also be implicit synchronization if two streams cannot run concurrently, for example, a page-locked host memory allocation or a device memory allocation is done on the host thread in-between the streams.


## Multiple GPUs

The other way to achieve concurrency is to have more than one GPUs. With the high demand of emerging applications including Large Language Model (LLM), etc, we need more CUDA cores and more GPU memory. To do that, we need to scale the GPU so they can provide a higher throughput and support larger applications. A multi-GPU system can be built in a different ways, which are connected with inter-connects. For example, Nvidia provides NVLink and NVSwitch to connect their own GPUs.

![Multiple GPUs](../../assets/unit-9/multigpu.png)

_**Figure 5:** Multiple GPUs_  
{: style="color:gray; font-size: 90%; text-align: center;" }

<iframe width="560" height="315" class="center" src="https://www.youtube.com/embed/RBf8FLS6q8E?si=5dYmDaWUQVzL6d2n" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" allowfullscreen></iframe><br/>

You can find more information on NVLink and NVSwitch can be found [here](https://www.nvidia.com/en-gb/data-center/nvlink/).


One of the examples to utilize multiple GPUs is the OpenMP-CUDA framework, which has a two-level scheme of parallelization:

![OpenMP-CUDA](../../assets/unit-9/openmp-cuda.png)

_**Figure 6:** (a) The two-level scheme of parallelization with OpenMP–CUDA. (b) Archi-tecture 2×CPU+6×GPU._  
{: style="color:gray; font-size: 90%; text-align: center;" }


Below is another example of TensorFlow to use multiple GPUs in training a Neural Network. The gradient loss are aggregated and used to update the model parameters:

![TesnsorFlow with Multiple GPUs](../../assets/unit-9/tensorflow-multi-gpu.png)

_**Figure 7:** TesnsorFlow with Multiple GPUs_  
{: style="color:gray; font-size: 90%; text-align: center;" }


* * *

# Advanced Topics on Memory
## Shared Memory

Recall the memory structure in CUDA, we have local, shared and global memory, each of which can be accessed by different groups of threads:

- Local memory
    - local thread only
- Shared memory
    - threads in block
- Global memory
    - all threads

Shared memory has much less latency than the global memory, and can be accessed by all the threads in a block. To use the shared memory, we rely on the following:

- `__shared__`: Annotation that denotes shared memory
- `__syncthreads()`: Synchronizes all threads __in a block__


An example of using (static) shared memory is given below, for a reverse function:

```c
__global__ void Reverse(int *d, int n)
{
  __shared__ int s[64];
  int t = threadIdx.x;
  int tr = n-t-1;
  s[t] = d[t];
  __syncthreads();
  d[t] = s[tr];
}
```

As many threads are accessing the shared memory at the same, to avoid conflict, memory is divided into **banks**, which is essential to achieve high bandwidth. Each bank can service one address per cycle, and thus a memory can service as many simultaneous accesses as it has banks. However, multiple simultaneous accesses to a bank will result in a **bank conflict**. In which case, these conflicting accesses are serialized.

When there is free of bank conflicts, memory access can be run in full parallel:

![bank-conflicts-free](../../assets/unit-9/bank-conflicts-free.png)

_**Figure 8:** Bank conflicts free_  
{: style="color:gray; font-size: 90%; text-align: center;" }


However, if two or more threads are access the same bank, bank conflicts will occur. To give an example, look at the diagram below. In memory access pattern, it is clearer that two threads in a warp access a same bank memory location like thread 0 access bank 0 or thread 8 access bank 0 and so on. So, clearly these requests become serialize, known as 2-Way bank conflict.

![bank-conflicts](../../assets/unit-9/bank-conflicts.png)

_**Figure 9:** 2-Way bank conflicts_  
{: style="color:gray; font-size: 90%; text-align: center;" }


For devices of compute capability 2.0, the warp size is 32 threads and the number of banks is also 32. Devices of compute capability 3.x have configurable bank size, which can be set using `cudaDeviceSetSharedMemConfig()` to either four bytes (`cudaSharedMemBankSizeFourByte`, the default) or eight bytes (`cudaSharedMemBankSizeEightByte`). Setting the bank size to eight bytes can help avoid shared memory bank conflicts when accessing double precision data.

Here is a very useful YouTube video on shared memory and bank conflicts by Peter Messmer from NVIDIA.

<iframe width="560" height="315" class="center" src="https://www.youtube.com/embed/qOCUQoF_-MM?si=iDnSUBexiMt3ktEp" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" allowfullscreen></iframe><br/>



## Memory Coalescing

In CUDA, memory accesses are called `coalesced` if all 32 threads in a warp access a contiguous chunk of memory. The following image shows coalesced memory access by all threads of a warp.

![Memory Coalescing](../../assets/unit-9/memory-coalescing.png)

_**Figure 10:** Memory Coalescing_  
{: style="color:gray; font-size: 90%; text-align: center;" }

Coalesed memory accessing can benefit from caching and reduce bank conflicts. On the other hand, if the memory is un-coalesced, potentially it will issue 32 sequential loads, which would largely reduce the memory performance.


## Unified Memory

From *Compute Capability 3.0+* and *CUDA 6.0+*, NVIDIA introduced a concept called *unified memory*:

- Unified Memory creates a pool of managed memory that is shared between the CPU and GPU and accessible to both using a single pointer.
- Data is on both GPU and CPU and the system automatically migrates data allocated in Unified Memory between host and device.
- This incurs small overhead.

You can allocate unified memory using the `cudaMallocManaged()` function. For example,

```c
int main(int argc, char *argv[]) {
    float *data;
    cudaMallocManaged(&data, dataSize * sizeOf(float));
    ...
    cudaFree(data);
    ...
}
```

More information on unified memory can be found [here](https://developer.nvidia.com/blog/unified-memory-cuda-beginners/).


# Profling and Performance Tuning
## Time a CUDA program

CUDA program can be timed with high precision CPU timers, e.g., `clock_gettime()`. However, as CUDA functions run asynchronously with the CPU, the timer on CPU is not always accurate (in terms of capturing the actual starting and completion points). To time a CUDA program in a more effective way, there is a built-in timer for events that can be utlized for this purpose.

The event timer measures the elapsed time between two events. An event can be created with `cudaEventCreate()` :

```c
cudaEvent_t start, stop;
cudaEventCreate(&start);
cudaEventCreate(&stop);
```

and destroried with `cudaEventDestroy()`:

```c
cudaEventDestroy( start );
cudaEventDestroy( stop );
```

After an event is created, one can record the event with `cudaEventRecord()`:

```c
cudaEventRecord (cudaEvent_t event, cudaStream_t stream = 0)
```

Below is an example of how to time code using `cudaEventElapsedTime()`:

```c
cudaEvent_t start, stop;
float time;

cudaEventCreate(&start);
cudaEventCreate(&stop);

cudaEventRecord( start, 0 );
kernel<<<grid,threads>>> ( d_odata, d_idata, size_x, size_y,
                           NUM_REPS);
cudaEventRecord( stop, 0 );
cudaEventSynchronize( stop );

cudaEventElapsedTime( &time, start, stop );
cudaEventDestroy( start );
cudaEventDestroy( stop );
```

## More on Profiling with nvprof

The nvprof profiling tool enables you to collect and view profiling data from the command-line. nvprof enables the collection of a timeline of CUDA-related activities on both CPU and GPU, including kernel execution, memory transfers, memory set and CUDA API calls and events or metrics for CUDA kernels. Profiling options are provided to nvprof through command-line options. Profiling results are displayed in the console after the profiling data is collected, and may also be saved for laterviewing by either nvprof or the Visual Profiler.

To use nvprof, simply in a new command line:

```
nvprof [options] [application]
    [application-arguments]
```

The nvprof command takes options, the name of the application, and the any parameters that need to be passed to the application. The most useful options for nvprof is `--print-gpu-trace`, which prints individual kernel invocations (including CUDA memcpy’s/memset’s) and sort them in chronological order. In event/metric profiling mode, show events/metrics for each kernel invocation.

Below is an example output form nvprof:

```
$ nvprof matrixMul
[Matrix Multiply Using CUDA] - Starting...
==27694== NVPROF is profiling process 27694, command: matrixMul
GPU Device 0: "GeForce GT 640M LE" with compute capability 3.0

MatrixA(320,320), MatrixB(640,320)
Computing result using CUDA Kernel...
done
Performance= 35.35 GFlop/s, Time= 3.708 msec, Size= 131072000 Ops, WorkgroupSize= 1024 threads/block
Checking computed result for correctness: OK

Note: For peak performance, please refer to the matrixMulCUBLAS example.
==27694== Profiling application: matrixMul
==27694== Profiling result:
Time(%)      Time     Calls       Avg       Min       Max  Name
 99.94%  1.11524s       301  3.7051ms  3.6928ms  3.7174ms  void matrixMulCUDA<int=32>(float*, float*, float*, int, int)
  0.04%  406.30us         2  203.15us  136.13us  270.18us  [CUDA memcpy HtoD]
  0.02%  248.29us         1  248.29us  248.29us  248.29us  [CUDA memcpy DtoH]

==27964== API calls:
Time(%)      Time     Calls       Avg       Min       Max  Name
 49.81%  285.17ms         3  95.055ms  153.32us  284.86ms  cudaMalloc
 25.95%  148.57ms         1  148.57ms  148.57ms  148.57ms  cudaEventSynchronize
 22.23%  127.28ms         1  127.28ms  127.28ms  127.28ms  cudaDeviceReset
  1.33%  7.6314ms       301  25.353us  23.551us  143.98us  cudaLaunch
  0.25%  1.4343ms         3  478.09us  155.84us  984.38us  cudaMemcpy
  0.11%  601.45us         1  601.45us  601.45us  601.45us  cudaDeviceSynchronize
  0.10%  564.48us      1505     375ns     313ns  3.6790us  cudaSetupArgument
  0.09%  490.44us        76  6.4530us     307ns  221.93us  cuDeviceGetAttribute
  0.07%  406.61us         3  135.54us  115.07us  169.99us  cudaFree
  0.02%  143.00us       301     475ns     431ns  2.4370us  cudaConfigureCall
  0.01%  42.321us         1  42.321us  42.321us  42.321us  cuDeviceTotalMem
  0.01%  33.655us         1  33.655us  33.655us  33.655us  cudaGetDeviceProperties
  0.01%  31.900us         1  31.900us  31.900us  31.900us  cuDeviceGetName
  0.00%  21.874us         2  10.937us  8.5850us  13.289us  cudaEventRecord
  0.00%  16.513us         2  8.2560us  2.6240us  13.889us  cudaEventCreate
  0.00%  13.091us         1  13.091us  13.091us  13.091us  cudaEventElapsedTime
  0.00%  8.1410us         1  8.1410us  8.1410us  8.1410us  cudaGetDevice
  0.00%  2.6290us         2  1.3140us     509ns  2.1200us  cuDeviceGetCount
  0.00%  1.9970us         2     998ns     520ns  1.4770us  cuDeviceGet
```

This gives an overview of the performance, enabling us to understand which function spends more time than the others, and then locate the performance bottleneck. To have a finer granularity of profiling, you can maunally turn on/off profiling within an executable using the cuda runtime API (defined in `cuda_profiler_api.h`):

```
cudaProfilerStart()
// and
cudaProfilerStop()
```

With this, you can collect profile information only for a specific kernel:

```
#include <cuda_profiler_api.h>
...

cudaProfilerStart();
myKernel<<<...>>>(...);
cudaProfilerStop();
```

A full documentation on nvprof, including a full list of command line options, can be found on the [Profiler's Guide](https://docs.nvidia.com/cuda/profiler-users-guide/index.html#nvprof).

## Automatic Block Size Tuning

Finding the best kernel launch parameters is extremely important in terms of performance. There is a useful function in CUDA, `cudaOccupancyMaxPotentialBlockSize`, that heuristically calculates a block size that achieves the maximum occupancy. This value can be then used as a starting point for further manual optimization.

`cudaOccupancyMaxPotentialBlockSize` is defined in the `cuda_runtime.h` as:

```
template<class T>
__inline__ __host__ CUDART_DEVICE cudaError_t cudaOccupancyMaxPotentialBlockSize(
    int    *minGridSize,
    int    *blockSize,
    T       func,
    size_t  dynamicSMemSize = 0,
    int     blockSizeLimit = 0)
{
    return cudaOccupancyMaxPotentialBlockSizeVariableSMem(minGridSize, blockSize, func, __cudaOccupancyB2DHelper(dynamicSMemSize), blockSizeLimit);
}
```

the input parameters are:

- `minGridSize` = Suggested min grid size to achieve a full machine launch.
- `blockSize` = Suggested block size to achieve maximum occupancy.
- `func` = Kernel function.
- `dynamicSMemSize` = Size of dynamically allocated shared memory. Of course, it is known at runtime before any kernel launch. The size of the statically allocated shared memory is not needed as it is inferred by the properties of func.
- `blockSizeLimit` = Maximum size for each block. In the case of 1D kernels, it can coincide with the number of input elements.

Once the block size is obtained, one needs to compute the 2D/3D block dimensions from the 1D block size suggested by the API (as of CUDA 6.5), according to the problem size.

Below you can find a full example of the automatic tuning process:

```c
#include <stdio.h>

/************************/
/* TEST KERNEL FUNCTION */
/************************/
__global__ void MyKernel(int *a, int *b, int *c, int N)
{
    int idx = threadIdx.x + blockIdx.x * blockDim.x;
    if (idx < N) { c[idx] = a[idx] + b[idx]; }
}

/********/
/* MAIN */
/********/
void main()
{
    const int N = 1000000;

    int blockSize;      // The launch configurator returned block size
    int minGridSize;    // The minimum grid size needed to achieve the maximum occupancy for a full device launch
    int gridSize;       // The actual grid size needed, based on input size

    int* h_vec1 = (int*) malloc(N*sizeof(int));
    int* h_vec2 = (int*) malloc(N*sizeof(int));
    int* h_vec3 = (int*) malloc(N*sizeof(int));
    int* h_vec4 = (int*) malloc(N*sizeof(int));

    int* d_vec1; cudaMalloc((void**)&d_vec1, N*sizeof(int));
    int* d_vec2; cudaMalloc((void**)&d_vec2, N*sizeof(int));
    int* d_vec3; cudaMalloc((void**)&d_vec3, N*sizeof(int));

    for (int i=0; i<N; i++) {
        h_vec1[i] = 10;
        h_vec2[i] = 20;
        h_vec4[i] = h_vec1[i] + h_vec2[i];
    }

    cudaMemcpy(d_vec1, h_vec1, N*sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_vec2, h_vec2, N*sizeof(int), cudaMemcpyHostToDevice);

    float time;
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start, 0);

    cudaOccupancyMaxPotentialBlockSize(&minGridSize, &blockSize, MyKernel, 0, N);

    // Round up according to array size
    gridSize = (N + blockSize - 1) / blockSize;

    cudaEventRecord(stop, 0);
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&time, start, stop);
    printf("Occupancy calculator elapsed time:  %3.3f ms \n", time);

    cudaEventRecord(start, 0);

    MyKernel<<<gridSize, blockSize>>>(d_vec1, d_vec2, d_vec3, N);

    cudaEventRecord(stop, 0);
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&time, start, stop);
    printf("Kernel elapsed time:  %3.3f ms \n", time);

    printf("Blocksize %i\n", blockSize);

    cudaMemcpy(h_vec3, d_vec3, N*sizeof(int), cudaMemcpyDeviceToHost);

    for (int i=0; i<N; i++) {
        if (h_vec3[i] != h_vec4[i]) { printf("Error at i = %i! Host = %i; Device = %i\n", i, h_vec4[i], h_vec3[i]); return; };
    }

    printf("Test passed\n");

}
```

## Tips for Performance Considerations

> "Hide the latency" --- The secret of success towards a high-performant GPU program  
> {: .block-warning }

There are a lot of implementation details that heavily influence how well CUDA programs run on a GPU. To improve the performance, below are some tips to get you started:

- Kernel Launch Configuration:
    - Launch enough threads per SM to hide latency
    - Launch enough thread blocks to load the GPU
- Global memory:
    - Maximise throughput (the GPU has lots of bandwidth, use it effectively)
    - Use shared memory when applicable (over 1 TB/s bandwidth)
- GPU-CPU interaction:
    - Minimise CPU/GPU idling, maximise PCIe throughput
- Use analysis/profiling when optimising

GPUs typically provide *cheap FLOP/s*; but in order to make the most effective use of a GPU, the effort required may not be *cheap*.


* * *

# Higher Level GPU Programming

Programming with CUDA is considered as *low-level programming*. Although CUDA provides generality and more flexibility in terms of memory and execution, there is a bunch of *high-level programming languages/libraries/tools* that provide a simpler interface to program a GPU. These tools are often designed for a specific purpose, such as machine learning or signal processing, so they are more *application-oritented*:

- Linear Algebra
    - CuBLAS, MAGMA,CUTLASS, Eigen, CuSPARSE, …
- Signal Processing
    - CuFFT, ArrayFire, …
- Deep Learning
    - CuDNN, TensorRT, …
- Graphics
    - OpenCV, FFmpeg, OpenGL, …
- Algorithms and Data Structures
    - Thrust, Raja, Kokkos, OpenACC, OpenMP, ...

Note that some of these are part of the CUDA toolchain (libraries with name starting with *Cuxx*), and some are using CUDA as the backend (e.g., OpenCV, TensorRT). If you want to move a step further into the world of GPU programming, pick one of these, do some research, and try to write/run some code!

* * *

# Recommended Reading

> - [CUDA C++ Programming Guide](https://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html), NVIDIA.
> - [CUDA C++ Best Practices Guide](https://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html), NVIDIA.
> - [Unified Memory for CUDA Beginners](https://developer.nvidia.com/blog/unified-memory-cuda-beginners/), Mark Harris, NVIDIA Technical Blog
> - [Profiler User’s Guide](https://docs.nvidia.com/cuda/profiler-users-guide/index.html), NVIDIA.
> - [NVLink and NVSwitch](https://www.nvidia.com/en-gb/data-center/nvlink/ "https://www.nvidia.com/en-gb/data-center/nvlink/#"), NVIDIA.  
>     {: .block-warning }
