---
layout: home
title: High Performance Parallel and Distributed Systems
permalink: /
---

Welcome to HIPC
================

Welcome to High Performance and Distributed Systems

In this module we will cover:
* The history of supercomputing and scientific computing
* Distributed computer architectures
* Management of distributed systems
* On-core, on-node and inter-node parallelism
* Accelerated architectures
* Heterogeneous programming

The module will be delivered asychronously through this VLE site, and will be supported by a 2-hour practical in a computer lab each week. The lecture content is divided into 8 units, and these units will consist of written content, video content and recommended reading/viewing (through text books, academic papers, youtube videos, etc). Each unit will appear below and will be segmented by topic.

If you have any questions about the module, please direct them to steven.wright@york.ac.uk

Unit 1 - Supercomputing and Scientific Computing
=================================================

In this unit we will put all of the material in the course into a historical context.

Specifically, this unit will cover:
* Scientific computing and its origins
* The history of supercomputing
* The basic laws of parallel computing
* The principles of performance engineering and scientific computing

Unit 2 - The Architectures of Distributed Systems
==================================================

In this unit we will cover the architectures of distributed systems from history up to modern day architectures.

We will cover:
* von Neumann architectures
* Modern CPU designs
* High speed interconnects
* Distributed file systems
* Computational accelerators

Unit 3 - Management and Measurement of Distributed Systems
===========================================================

This unit will cover all aspects of the management of distributed systems, and the measurement of the performance of distributed systems

Specifically, we will explore:
* Operating systems for distributed systems
* Environment management
* HPC scheduling systems
* Performance measurement and analysis

Unit 4 - On-Core Parallelism
=============================

In this unit we will begin to discuss the parallelisation of applications on a system. We will begin at the base level, looking at parallelisation within a single core.

This unit will cover:
* Flynn's taxonomy
* Simple optimisations
* Less simple optimisations
* Vectorisation
* Auto-vectorisation

Unit 5 - On-node Parallelism 
=============================

Last week we covered how we might parallelise an application through a SIMD approach. This week we will cover how we might achieve parallelism on a multi-core CPU using threading.

We will cover:
* Single-Program Multiple Data (SPMD)
* pthreads
* OpenMP

Unit 6 - Inter-node Parallelism
================================

We're now ready to move our applications to a distributed system.

In this unit, we will cover:
* Shared Memory Systems
* Message Passing
* The MPI Library

Unit 7 - Accelerators
======================

In 2008 Roadrunner became the first supercomputer to break the PetaFLOP/s barrier. Roadrunner was perhaps the first modern heterogenous system, with each node employing PowerXCell accelerators to achieve high performance. Today, many of the largest systems in the world are heterogeneous platforms, employing GPUs to accelerate their computational workloads. This unit covers the basics of accelerators and how to program them.

Specifically, we will cover:
* Historical context of Accelerators in HPC
* GPGPU Platforms
* GPU Models
* CUDA Programming

Unit 8 - Heterogenous Programming
==================================

This module has introduced many layers of parallelism across a system. Exploiting each platform relies on specific programming models that do not translate easily between platforms -- each new platform may require a porting effort.

In this final unit we will cover approaches to heterogeneous programming and performance portability.
Specifically, we will explore:
* Assessing portability and performance portability
* The SYCL and DPC++ languages
* Alternative approaches (Kokkos, RAJA, etc)