---
title: 9. Heterogeneous Programming 
date: 2022-07-21
category: hipc
layout: post
---

> **This practical is a work-in-progress**
{: .block-danger }

# Overview

In this practical, we're going to start working on targetting multiple heterogeneous architectures from a single-source application. We're going to do this through two approaches

> # Exercise 1
>
> In Unit 6, we discussed collective operations such as reductions. There was an exercise in the unit to implement a broadcast using point-to-point communications. For an operation such as an `MPI_Reduce`, this could be a significant bottleneck.
>
> Implement your own reduction function using point-to-point operations, but make use of non-blocking functions, and write the application such that the order of operations does not matter.
>
> Extend your application to include an Allreduce equivalent (i.e. broadcast the result).
>
> **Hint**: It is significantly easier to focus on a single data type and a single reduction operation. For example, create a reduction that performs the `MPI_SUM` operation on an array of doubles. Extending it to accept generic operations and generic data types is much more difficult.<br/><br/>
{: .block-danger }

SOME OPENMP WITH OFFLOAD

LOAD NVIDIA C++ COMPILER ON VIKING:
NVHPC/23.7-CUDA-12.1.1

Get job on GPU nodes: srun --time=00:10:00 --partition=gpu --gres=gpu:1 --mem=10G --pty /bin/bash



SOME SYCL

intel-compilers/2023.1.0