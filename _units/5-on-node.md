---
title: 5. On-node Parallelism
date: 2022-07-25
category: Jekyll
layout: post
---

# Efficient OMP

Before we cover some tips and tricks to make the best use of OpenMP, you might like to learn more about OpenMP straight from one of its originators, Tim Mattson, from Intel.

[Introduction to OpenMP](https://youtube.com/playlist?list=PLLX-Q6B8xqZ8n8bwjGdzBJ25X2utwnoEG) (The Youtube playlist prevents me from embedding these videos)

## Performance pitfalls

Like any other method of parallelising code, OpenMP is prone to a number of performance issues, such as load balancing or serial fraction (see Amdahl's law, Unit 2). However, we can sometimes alleviate these issues. This section will cover some of the issues that might appear and how we can potentially mitigate them.

### Avoid unnecessary parallelisation

Any parallel region of code has an associated cost as well as a benefit. Upon encountering a parallel region, the runtime will incur a penalty in either spawning threads, or in waking them from an idle state. If the work being parallelised is suitably small, it may be the case that this cost outweights the benefit. For these instances, there are a few potential solutions.

Firstly, we could avoid running a region in parallel if its unlikely to pay off. We can control this with the if clause, restricting our parallel region to a certain condition. For example,

```c
int iters = 10;
#pragma omp parallel for if (iters > 100)
for (int i = 0; i < iters; i++) {
    printf("Thread %d, running iteration %d\n", omp_get_thread_num(), i);
}
```

In the code above, the code will only parallelise if there are more than 100 iterations to complete. The point at which parallelisation may become beneficial will be problem- and platform-specific, but you could explore this space with profiling (see Unit 3).

Alternatively, we could manually reduce the number of threads such that the overhead is minimised. We can do this by controlling the number of threads with a `num_threads` clause. E.g.

```c
int iters = 10;
#pragma omp parallel for num_threads(2)
for (int i = 0; i < iters; i++) {
    printf("Thread %d, running iteration %d\n", omp_get_thread_num(), i);
}
```

In this example, regardless of the number of threads available to the application, this loop will only ever use 2. Less threads means less overhead, and thus this might improve performance of a loop, without slowing down later loops that may benefit from more threads.

###ß Avoid implicit barriers

As was mentioned in the previous section, there are implicit barriers at the end of all parallel regions in OpenMP. This means that if some threads finish their work early, they will block until all threads have finished. In cases there this is not required, we can instruct threads not to wait and to continue their execution.

There is an implicit barrier at the end of any parallel region that cannot be removed, but we can remove the implicit barrier from a work-sharing construct with the `nowait` clause. Consider the `omp for pragma` in the following code sample:

```c
int iters = 10;
#pragma omp parallel 
{
    #pragma omp for 
    for (int i = 0; i < iters; i++) {
        printf("Thread %d, running iteration %d\n", omp_get_thread_num(), i);
    }
    printf("Thread %d is done\n", omp_get_thread_num());
}
```

The output for this shows that each thread operates in lockstep, with each thread waiting at the end of the for-loop before executing the final printf().

```bash
$ OMP_NUM_THREADS=5 ./demo            
Thread 1, running iteration 2
Thread 1, running iteration 3
Thread 2, running iteration 4
Thread 2, running iteration 5
Thread 0, running iteration 0
Thread 0, running iteration 1
Thread 3, running iteration 6
Thread 3, running iteration 7
Thread 4, running iteration 8
Thread 4, running iteration 9
Thread 1 is done
Thread 2 is done
Thread 0 is done
Thread 3 is done
Thread 4 is done
```

If we add the `nowait` clause to our `omp for` pragma, the output shows that we have removed one of the implicit barriers in our code.

```shell
$ OMP_NUM_THREADS=5 ./demo            
Thread 1, running iteration 2
Thread 1, running iteration 3
Thread 1 is done
Thread 3, running iteration 6
Thread 3, running iteration 7
Thread 4, running iteration 8
Thread 4, running iteration 9
Thread 0, running iteration 0
Thread 0, running iteration 1
Thread 0 is done
Thread 4 is done
Thread 3 is done
Thread 2, running iteration 4
Thread 2, running iteration 5
Thread 2 is done
```

Before adding the `nowait` clause to a work sharing construct, its important that you check that it is safe to do so!

### Avoid trivial load imbalance

In order to get the best out of OpenMP, the number of loop iterations should be large compared to the number of threads. If we have a small number of iterations spread over a similarly small number of threads (even if iterations does outnumber threads slightly), we are likely to end up with load imbalance.

Say for example, we have a triply nested loop where the outermost loop has been parallelised and does M iterations on N threads. In the case where M is greater than N, but less than 2N, we will have some threads performing two iterations, and some threads performing only one iteration (i.e. half the work). This will result in a significant waste of resourses and performance.

For loop nests like this, we may be able to make use of the `collapse` clause to combine multiple levels of loop.

So, for example,

```c
#pragma omp parallel for collapse(2)
for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++) {
        for (int k = 0; k < O; k++) {
            ...
        }
    }
}
```

Here the two outermost loops are collapsed into a single loop of length M x N, that can be executed in parallel. The collapse keyword is specific to perfect loop nests, i.e., loop nests where there is no code between them, and the loop counts do not depend on each other.

### Avoid dynamic/guided loop scheduling or tasking unless necessary

All parallel work-sharing scheduling options (except static) and tasking constructs require some amount of nontrivial computation or bookkeeping. If there is only a small amount of work to do, it is likely that the overhead will dominate the performance improvement that can be gained. Think carefully before using these features, and consider (and benchmark) the alternatives!