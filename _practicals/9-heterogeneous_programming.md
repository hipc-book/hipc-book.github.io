---
title: 9. Heterogeneous Programming 
date: 2022-07-21
category: hipc
layout: post
---

# Overview

In this practical, we're going to start working on targetting multiple heterogeneous architectures from a single-source application. We're going to do this through two approaches: OpenMP with target offload directives and SYCL/Data Parallel C++.

> Both exercises this week are based on the [heat equation code](../../assets/practical-10/heat.c) from Practical 6. However, in the code linked here, the 2D array has been flattened for you.
{: .block-tip }

# OpenMP with Target Offload Directives

We've covered OpenMP in earlier units and practicals, showing how we can achieve better performance through scheduling iterations of loops across multiple CPUs. In this practical, we're going to look at how we can do a similar thing using all of the parallelism available on accelerator devices.

## Using a Compliant Compiler and the Right Hardware

To complete this practical, we're going to need the right hardware and software -- that is to say, we're going to need a compiler with support for target offload directives, and we're going to need hardware it is capable of targetting.

The best OpenMP 4.0+ compliant compiler available on Viking (with the ability to target NVIDIA GPUs) is the NVHPC compiler from the NVIDIA HPC SDK. We can load it on Viking like so:

```
$ module load NVHPC/23.7-CUDA-12.1.1
```

We can then compile with OpenMP using the `nvc` binary and using the `-mp` (or `-mp=gpu` to target GPUs) option. For diagnostic information, we can pass the `-Minfo=mp` option.

```
$ nvc -mp=gpu -Minfo=mp -o test test.c 
main:
     65, Generating target enter data map(create: data[:100])
	 69, #omp target teams loop
         69, Generating "nvkernel_main_F1L69_2" GPU kernel
             Generating NVIDIA GPU code
	 ...
```

One important thing to note is that the NVIDIA HPC compiler with target offload directives can only target GPUs with Compute Capability 7.0 and above. This means that you cannot use this compiler with the GPUs prior to the Volta range (meaning we cannot use the GPUs in the labs).

For this reason, we should instead use Viking's GPU nodes, that contain A40s (Ampere) or H100s (Hopper) GPUs. You can request an interactive session on one of these nodes like so:

```
$ srun --time=00:10:00 --account=CS-TEACH-2023 --partition=gpu --gres=gpu:1 --mem=10G --pty /bin/bash
```

## Prescriptive Parallelism vs. Descriptive Parallelism

Up to now, we've been using a prescriptive model of OpenMP parallelism, explicitly specifying every action to be taken by an OpenMP compiler.  

Using target offload semantics, we can expand upon our typical list of pragmas to express parallelism for offloading to a GPU.

```c
#pragma omp target       // map variables to a device and execute on that device
#pragma omp teams        // create a leaug of teams where initial thread of each team executes the region
#pragma omp parallel     // create team of parallel threads to execute a region
#pragma omp distribute   // distribute loop iterations across teams
#pragma omp for          // distribute loop iterations across threads
...
``` 

So for example we could distribute a loop over GPU teams and threads like so:

```c
double total = 0.0;

#pragma omp target teams distribute parallel for reduction(sum:total) collapse(2)
for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
        total += A[i][j]
    }
}
```

We could potentially restructure our code to achieve more parallelism by using teams and threads in a hierarchical manner, like so:

```c
double total = 0.0;

#pragma omp target teams distribute reduction(sum:total)
for (int i = 0; i < N; i++) {
    #pragma omp parallel for reduction(sum:total)
    for (int j = 0; j < N; j++) {
        total += A[i][j]
    }
}
```

However, doing this may harm performance on a CPU (since now the outer loop will not be parallelised if the loop is not being executed on an accelerator target).

We could instead write our application in a more _descriptive_ manner, where instead of being specific about how parallelism is achieved, we describe where parallelism exists and allow the compiler to do the hard work (depending on the target platform).

The best example of this style of programming is the `omp loop` directive. This directive is more portable, allowing the compiler to assign parallelism as it sees fit. So for example:

```c
double total = 0.0;

#pragma omp target teams loop reduction(sum:total)
for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
        total += A[i][j]
    }
}
```

Using this directive, you will find that the NVHPC compiler will create multiple versions of the same kernel for different devices. For example:

```
     69, #omp target teams loop
         69, Generating "nvkernel_main_F1L69_2" GPU kernel
             Generating NVIDIA GPU code
           73, Loop parallelized across teams, threads(128) collapse(2) /* blockIdx.x threadIdx.x */
           74,   /* blockIdx.x threadIdx.x collapsed */
         69, Generating Multicore code
           73, Loop parallelized across threads
```

Here we can see that for the GPU it has created a loop parallelised across teams and threads, while for the multicore kernel it is parallelised only across threads.

> **Further Reading**
>
> * [Thinking OpenMP with NVIDIA HPC Compilers](https://www.nas.nasa.gov/assets/nas/pdf/ams/2021/AMS_20210504_Ozen.pdf), Güray Özen, NVIDIA
> * [Best Practices for OpenMP](https://www.olcf.ornl.gov/wp-content/uploads/nersc_best_practices_sep_1_2022.pdf), Chris Daley, NERSC
{: .block-tip }

> # Exercise 1
>
>  Your first task is to parallelise the `heat` code using OpenMP target offload directives. You should insert timers so that you can time the main compute loop of the code, and then analyse the speed-up you can achieve using a GPU.
>
> **Hint:** When offloading compute, you need to carefully consider data movement to and from device. There are tips on this in Unit 10.<br/><br/>
{: .block-danger }

# SYCL/Data Parallel C++

Another approach to single-source portable code is SYCL. The SYCL standard has been adopted by Intel for its new Xe GPU line, and is supported by all major GPUs through various compilers and frameworks.

On Viking, SYCL is supported by the Intel Compiler. Unfortunately, this version of the compiler does not support the NVIDIA GPUs on Viking, but nonetheless we can test the principles of SYCL with this compiler.

You can load the Intel compiler like so:

```
$ module load intel-compilers/2023.1.0
```

This will give us access to the Intel C/C++ compiler, and we can compile SYCL code with the following:

```
$ icpx -fsycl ...
```

## The SYCL Programming Model

For the most part (for our problem at least), the SYCL programming model is similar to the OpenMP programming, with each of our main compute loops being annotated or replaced with parallelised versions of the loop. However, there are some complexities to consider.

### Nested Loops

In OpenMP, we can collapse perfectly nested loops with the `collapse(N)` directive. In SYCL we can achieve a similar effect using a `sycl::range` with up to three dimensions, and using multi-dimensional indices. So for example,

```c++
for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++) {
        // do compute on data[i][j]
    }
}
```

becomes,

```c++
myqueue.submit([&](sycl::handler& h) {
    sycl::accessor data{ data_buf, h, sycl::read_write };				
    h.parallel_for(sycl::range{M, N}, [=](sycl::id<2> idx) {
        int i = idx[0];
        int j = idx[1]; 
        // do compute on data[i][j]
    });
}).wait();
```

In this example, we define our execution range to be `M` in one dimension and `N` in the second dimension, and we retrieve these indices through the multi-dimensional `idx` variable.

**Note:** In SYCL the indices _must_ start at 0. If you want to offset your loops, you should do this computation internally. So for example, if you want to loop between 1 and M-1, you would use `sycl::range{ M-2 }`, and then use `idx+1`.

### Reductions

The other complexity in the heat code lies in the final loop structure, calculating the greatest difference between the previous solution and the current solution. It is this computation that allows us to know when we have reached some "convergence criteria".

Reductions in SYCL are somewhat more complex than in OpenMP, but nonetheless, they can be handled by the language (rather than manually as in CUDA).

Within a looping structure, we can define a `sycl::reduction` that specifies a preset or a custom "combiner" to perform a reduction. For example,

```c++
double sum = 0.0;
sycl::buffer<double> sum_b { &sum, 1 };

myqueue.submit([&](sycl::handler& h) {
    sycl::accessor data{ data_buf, h, sycl::read_write };				
    h.parallel_for(sycl::range{ M }, sycl::reduction(sum_buf, h, sycl::plus<>()), [=](sycl::id<1> idx, auto& sum) {
        int i = idx[0];
        sum += data[i];
    });
}).wait();
```

As with OpenMP and MPI there are a number of pre-defined reduction operations: `sycl::plus`, `sycl::multiplies`, `sycl::bit_and`, `sycl::bit_or`, `sycl::bit_xor`, `sycl::logical_and`, `sycl::logical_or`, `sycl::minimum`, and `sycl::maximum`. The results from a reduction operation are typically combined with the `combine()` function, but some operators are overloaded for convenience (such as `+=` above). When this isn't available, we can instead do:

```c++
...
        sum.combine(data[i]);
...
```

> # Exercise 2
>
> Your second task is to parallelise the `heat` application using SYCL. Again, use timers to analyse any speed-ups.
>
> **Hint:** Since we will likely be dealing with buffers and accessors, it might be worth using a C++ data structure for our data (and renaming the file to `.cpp`!). You can replace the `malloc` lines with `std::array`, so that we can pass these directly to SYCL. For example,
>
> ```c++
> std::array<double, M * N> u;
> ...
> sycl::buffer u_buf { u };
> ...
> sycl::accessor u { u_buf, h, sycl::read_only };
> ```
>
> **Beware:** Because we cannot execute our code on a GPU we may not see any speed up from SYCL right now (and in fact, we may see slow down through the OpenCL library).<br/><br/>
{: .block-danger }
			
