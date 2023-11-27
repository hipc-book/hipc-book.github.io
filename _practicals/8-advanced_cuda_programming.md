---
title: 8. Advanced CUDA Programming
date: 2022-07-21
category: hipc
layout: post
---


# Overview

In this lab, we will look at advanced CUDA topics including shared memory and asynchronous execution. We will also look at how to profile and fine-tune the performance of a parallel GPU program. If you haven't done the previous lab, you should work on that first as the topics here are considered to be more _advanced_.

Good luck and enjoy!

# Matrix Multiplication

Consider a matrix-matrix multiplication problem, i.e., $C = A \times B$.
To multiply an $m \times n$ matrix ($A$) by an $n \times p$ matrix ($B$), the $n$s must be the same, and the result is an $m \times p$ matrix ($C$).

![Matrix Multiplication](../../assets/practical-8/matrix-multiplication.png)  
_**Figure 1:** Matrix Multiplication_
{: style="color:gray; font-size: 90%; text-align: center;" }

The matrix multiplication can be calculated with the following equation:

$$
C_{i,j} = \sum_k A_{i,k}B_{k,j}
$$

Based on this equation, we can start by writing a very basic serial version of matrix multiplication:

```c
void matrix_multiplication(double *A, double *B, double *C, int M, int N, int P) {
  // initialization C with zeros
  for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++)
      C[i][j] = 0;

  for (int i = 0; i < M; i++)
  	for (int j = 0; j < N; j++)
  		for (int k = 0; k < P; k++)
  			C[i][j] += A[i][k]*B[k][j];
}
```


> # Exercise 1
> Rewrite the above code in CUDA with 1D thread blocks. Replace the function with a kernel and write a `main` function to launch that kernel function. At this first stage, make it as simple as possible. Later on, this will be used as the baseline.
>
> Note that you need to design a verification process to validate the results are correct.
{: .block-danger }



> # Exercise 2
> Time the program you have written in Exercise 1 using `cudaEventElapsedTime()`. Then profile your code with `nvprof`. These should give you similar if not identical results.
{: .block-danger }

## Optional Exercise

One of the additional profiling tools that we didn't mention in the unit is the [Visual Profiler](https://docs.nvidia.com/cuda/profiler-users-guide/index.html#visual-profiler) which allows you to analyse and visualise the performance of your application. The Visual Profiler gives you a different _view_ that helps you to understand the CPU and GPU activities. You may find it is functionally similar to Intel Advisor.

For example, a Timeline view shows GPU events with elapsed time:

![Timeline View](../../assets/practical-8/timeline-view.png)  
_**Figure 2:** Timeline View in Visual Profiler_  
{: style="color:gray; font-size: 90%; text-align: center;" }

The other useful view is the Analysis View, which is used to control application analysis and to display the analysis results. Two analysis modes exist in Analysis View, which are guided and unguided:
- Guided mode: the analysis system will guide you through multiple analysis stages to help you understand the likely performance limiters and optimization opportunities in your application.
- Unguided mode: let you manually explore all the analysis results collected for your application.

![Analysis View](../../assets/practical-8/analysis-view.png)  
_**Figure 3:** Analysis View in Visual Profiler_
{: style="color:gray; font-size: 90%; text-align: center;" }

> # Exercise 3
> Rewrite your code with 2D kernels. After you have done that, use automatic block size tuning (`cudaOccupancyMaxPotentialBlockSize(...)`). Note that this only gives a block size and you have to work out your own 2D/3D block dimension to match your problem size.
>
> Plot the performance against different parameters and see how automatic tuning performs. You can first collect the performance data, then use MATLAB, Python with matplotlib, or Excel to plot the results.
{: .block-danger }

Occupancy is an important concept to achieve performant CUDA programs, you can refer to the [CUDA Occupancy Calculator](https://xmartlabs.github.io/cuda-calculator/) as it would give you some insight into what to consider in terms of performance tuning.

> # Exercise 4
> Now try to improve your kernel function with Shared Memory.
>
> **Tips:**
> - Decompose the problem: each block computes a submatrix (illustrated as below).
> - Make sure that data is only loaded once from the main memory and then stored in shared memory.
> - Use `__syncthreads()` where appropriate to avoid race condition.
>
> ![Sub-Matrix Multiplication](../../assets/practical-8/sub-matrix-multiplication.png)  
> _**Figure 4:** Sub-Matrix Multiplication_
> {: style="color:gray; font-size: 90%; text-align: center;" }
{: .block-danger }


> # Exercise 5
> Profile different versions of your implementations. Plot the results and make a comparison of their performance.
{: .block-danger }

> **Further Reading**
>
> - [CUDA C++ Best Practices Guide](https://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html), NVIDIA.
> - [Profiler User’s Guide](https://docs.nvidia.com/cuda/profiler-users-guide/index.html), NVIDIA.
{: .block-tip }
