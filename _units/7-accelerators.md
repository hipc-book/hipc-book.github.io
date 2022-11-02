---
title: 7. Accelerators
date: 2022-07-27
category: hipc
layout: post
---

# Overview

<video width="560" class="center" controls>
    <source src="/hipc/assets/videos/HIPC-Unit_7-Overview.mp4" type="video/mp4">
</video><br/> 
 
Welcome to the 7th unit of the HIPC course. 
  
In 2008 Roadrunner became the first supercomputer to break the PetaFLOP/s barrier. Roadrunner was perhaps the first modern heterogenous system, with each node employing PowerXCell accelerators to achieve its high performance. Today, many of the largest systems in the world are heterogeneous platforms, employing GPUs to accelerate their computational workloads. This unit covers the basics of accelerators and how to program them. 

Specifically, we will cover: 

* Accelerators in HPC  
* GPGPU platforms  
* GPU models  
* Programming with CUDA  

After this unit, you are expected to have a better understanding of GPU and CUDA programming, and be able to write CUDA programs.
  
# A Brief Introduction to GPUs in HPC
     
An _accelerator_ is, as the name indicates, a device to speed up certain computations in an HPC application. Accelerators are peripheral processors that are able to perform additional work in the background, while releasing the resources of the main processor.   

In the June 2016 TOP500 list, 19% of state-of-the-art HPC systems used GPUs (graphics processing units) or other accelerators (e.g. FPGA-based HPC accelerators). In general, accelerators can be classified as (1) general-purpose, or (2) domain-specific. The use of accelerators involves the cooperative design of software and hardware (or SW & HW co-design), which sometimes makes porting and maintenance difficult, as the software and hardware are coupled for a specific software application and hardware platform. 

Accelerated computing started to gain popularity with the release of the first Petascale system, Roadrunner, which had IBM PowerXCell 8i accelerators connected to each core.

![Roadrunner's architecture](/hipc/assets/unit-6/)
_**Figure 1:** Roadrunner Architecture_
{: style="color:gray; font-size: 90%; text-align: center;" }
  
In the following years, the trend rapidly moved towards the use of GPUs, due to their versatility and relatively low cost. The prevalence of GPUs as accelerators has significantly improved the programmability issue due to their general-purpose nature and well supported programming infrastructure. These GPUs that can be used for calculation are sometimes referred to as _GPGPUs_ (General Purpose Graphic Processing Units).  

In 2007, NVIDIA released its _CUDA_ development environment, the earliest widely adopted programming model for GPU computing. Two years later, OpenCL became widely supported. The OpenCL framework allows for the development of code for both GPUs and CPUs with an emphasis on portability. Thus, GPUs became a more generalised computing device. 

Despite other competitors in the market (e.g. AMD, Intel), the combination of NVIDIA GPUs and CUDA dominates several application areas, including scientific computing, deep learning, animation rendering, and NVIDIA GPUs are the foundation for some of the fastest computers in the world.

CUDA, as already mentioned, is a parallel computing platform and programming model developed by NVIDIA for general computing on its own GPUs. CUDA enables developers to speed up compute-intensive applications by harnessing the power of GPUs for the parallelisable part of the computation. In this unit, we will focus on NVIDIA GPUs and CUDA programming, but many of the concepts will apply to other GPUs and GPU programming models.

## GPU vs CPU: What's the difference?
    
GPUs were originally designed to accelerate the rendering of 3D graphics. During the last few decades, several researchers have used GPUs to perform scientific computations, including fluid flow simulation using the Lattice Boltzman model, cloud dynamics simulation, finite-element simulations, ice crystal growth, etc. 

However, an obvious question to ask is why we need to use a GPU in addition to a CPU? Well, we know a CPU is good for general-purpose computing, however, it is optimised for serial tasks. On the other hand, a GPU is optimised for parallel tasks and it is extremely powerful at running smaller (and simpler) jobs simultaneously.  

Architecturally, in a GPU there are often hundreds of arithmetic logic units (ALUs). While for a CPU there are only a limited number of ALUs, usually correlated to the number of cores. This architectural difference leads to their different approaches to processing tasks, and therefore dictates which purposes they are good/bad for. 

![Comparison between CPUs and GPUs]()
_**Figure 2:** THe key differences between CPUs and GPUs_

To make it clearer, in the following table we list the strengths of a CPU and a GPU: 

<table class="table table-striped" cellpadding="8px" style="border-collapse: collapse; width: 1020px; background-color: #ffffff; margin-left: auto; margin-right: auto;"> 
  <thead> 
   <tr> 
    <th style="padding: 8px; text-align: left; vertical-align: bottom; border-top-width: 0px; border-bottom-width: 2px; border-bottom-style: solid; border-bottom-color: #dddddd; width: 524.234375px;">_CPU strengths_</th> 
    <th style="padding: 8px; text-align: left; vertical-align: bottom; border-top-width: 0px; border-bottom-width: 2px; border-bottom-style: solid; border-bottom-color: #dddddd; width: 494.78125px;">GPU strengths</th> 
   </tr> 
  </thead> 
  <tbody> 
   <tr style="background-color: #f9f9f9;"> 
    <td style="padding: 8px; vertical-align: top; border-top-width: 1px; border-top-style: solid; border-top-color: #dddddd; width: 524.234375px;">Very large main memory</td> 
    <td style="padding: 8px; vertical-align: top; border-top-width: 1px; border-top-style: solid; border-top-color: #dddddd; width: 494.78125px;">High bandwidth main memory</td> 
   </tr> 
   <tr style="height: 35px;"> 
    <td style="padding: 8px; vertical-align: top; border-top-width: 1px; border-top-style: solid; border-top-color: #dddddd; width: 524.234375px;">Very fast clock speeds</td> 
    <td style="padding: 8px; vertical-align: top; border-top-width: 1px; border-top-style: solid; border-top-color: #dddddd; width: 494.78125px;">Latency tolerant via parallelism</td> 
   </tr> 
   <tr style="background-color: #f9f9f9;"> 
    <td style="padding: 8px; vertical-align: top; border-top-width: 1px; border-top-style: solid; border-top-color: #dddddd; width: 524.234375px;">Latency optimized via large caches</td> 
    <td style="padding: 8px; vertical-align: top; border-top-width: 1px; border-top-style: solid; border-top-color: #dddddd; width: 494.78125px;">Significantly more compute resources</td> 
   </tr> 
   <tr style="height: 35px;"> 
    <td style="padding: 8px; vertical-align: top; border-top-width: 1px; border-top-style: solid; border-top-color: #dddddd; width: 524.234375px;">Small number of threads can run very quickly</td> 
    <td style="padding: 8px; vertical-align: top; border-top-width: 1px; border-top-style: solid; border-top-color: #dddddd; width: 494.78125px;">High throughput</td> 
   </tr> 
   <tr style="background-color: #f9f9f9;"> 
    <td style="padding: 8px; vertical-align: top; border-top-width: 1px; border-top-style: solid; border-top-color: #dddddd; width: 524.234375px;"> </td> 
    <td style="padding: 8px; vertical-align: top; border-top-width: 1px; border-top-style: solid; border-top-color: #dddddd; width: 494.78125px;">High performance/watt</td> 
   </tr> 
  </tbody> 
 </table> 
  
 In terms of weaknesses, a comparison is made below: 
 <table class="table table-striped" cellpadding="8px" style="border-collapse: collapse; width: 1020px; background-color: #ffffff; margin-left: auto; margin-right: auto;"> 
  <thead> 
   <tr> 
    <th style="padding: 8.0px; text-align: left; vertical-align: bottom; border-top-width: 0.0px; border-bottom: 2.0px solid #dddddd;">CPU weaknesses</th> 
    <th style="padding: 8.0px; text-align: left; vertical-align: bottom; border-top-width: 0.0px; border-bottom: 2.0px solid #dddddd;">GPU weaknesses</th> 
   </tr> 
  </thead> 
  <tbody> 
   <tr style="background-color: #f9f9f9;"> 
    <td style="padding: 8.0px; vertical-align: top; border-top: 1.0px solid #dddddd;">Relatively low memory bandwidth</td> 
    <td style="padding: 8.0px; vertical-align: top; border-top: 1.0px solid #dddddd;">Relatively low memory capacity</td> 
   </tr> 
   <tr> 
    <td style="padding: 8.0px; vertical-align: top; border-top: 1.0px solid #dddddd;">Low performance/watt</td> 
    <td style="padding: 8.0px; vertical-align: top; border-top: 1.0px solid #dddddd;">Low per-thread performance</td> 
   </tr> 
  </tbody> 
 </table> 

Modern CPUs strongly favour sequential serial processing with high operational frequency and get a boost from the large size of caches. The range of tasks they are appropriate for is wide, as the architecture is very general-purpose. However, a CPU is not great at everything, especially in the case of highly parallelisable programs. In some cases, GPUs can be 100x or more faster than CPUs with fine-grained parallelism. This makes GPUs a great candidate for offloading workloads that CPUs are not good at. 

A more direct comparison of their performance is given in the following diagram, where the single- and double-precision floating-point performance (left) and memory performance (right) of Nvidia Tesla 20 GPUs is compared with x86 CPUs: 

![]()
_Figure 3: _CPU and GPU performance comparison 

Finally, a very famous demo of the difference of GPU versus CPU is done by Mythbusters in 2009: 

<iframe width="640" height="480" src="https://www.youtube.com/embed/-P28LKWTzrI" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen=""></iframe> 

### The Microarchitecture of Nvidia GPUs
     
The architecture of Nvidia GPUs has been evolving for serval years. Since 2006, Nvidia has released a number of different GPU microarchitectures, which are: <a href="https://en.wikipedia.org/wiki/Tesla_(microarchitecture)" target="_blank" rel="noopener">Tesla</a> (2006), <a href="https://en.wikipedia.org/wiki/Fermi_(microarchitecture)" target="_blank" rel="noopener">Fermi</a> (2010), <a target="_blank" rel="noopener">Kepler</a> (2012), <a href="https://en.wikipedia.org/wiki/Maxwell_(microarchitecture)" target="_blank" rel="noopener">Maxwell</a> (2014), <a href="https://en.wikipedia.org/wiki/Pascal_(microarchitecture)" target="_blank" rel="noopener">Pascal</a> (2016), <a href="https://en.wikipedia.org/wiki/Volta_(microarchitecture)" target="_blank" rel="noopener">Volta</a> (2017), <a href="https://en.wikipedia.org/wiki/Turing_(microarchitecture)" target="_blank" rel="noopener">Turing</a> (2018) and <a href="https://en.wikipedia.org/wiki/Ampere_(microarchitecture)" target="_blank" rel="noopener">Ampere</a> (2020). 

An overview of Nvidia GPU architecture is given in the following diagram: 
  
![]()
_Figure 4:_ GPU Hardware Model -- Overview (A100) 
  
It can be seen that from the top level, a GPU is similar to a CPU with respect to the memory hierarchy. But when looking down into the low-level microarchitecture, a GPU is very different from how a CPU is organised and designed. 

An Nvidia chip consists of one or more _streaming multiprocessors_ (SMs). Each SM has a dedicated L1 cache, and all SMs share a unified L2 cache. An SM then has 1-4 _warp schedulers_. Each warp scheduler has a register file and multiple execution units. The execution units may be exclusive to the warp scheduler or shared between schedulers. Execution units include _CUDA cores_ (FP/INT), _special function units_ (SPU), _texture_, and _load-store units_ (LD/ST).   

In the following figure, we take the Pascal computing architecture (Geforce GTX 1080, Telsa P100, etc.) as an example to look inside an SM. The diagrammatic structure is shown below: 
  
![]()
_Figure 5:_ GPU Hardware Model -- SM (of a GP104/Pascal) 
  
As can be seen, the number of CUDA cores within an SM is huge. In a Geforce GTX 1080, there are 20 streaming multiprocessors (SM), each with 128 CUDA processor cores, for a total of 2560 cores. To efficiently manage and use this many cores, each SM uses _single-instruction multiple-thread_ (SIMT), where concurrent threads are created, managed, scheduled, and executed in a group of parallel threads, or _warps_. 

We will cover these in detail in the following section.  

### GPU Programming with CUDA
     
#### What is CUDA? 

CUDA (Compute Unified Device Architecture) is a parallel computing platform and application programming interface (API) that allows software to use certain types of the graphics processing unit (GPU) for general-purpose processing -- an approach called general-purpose computing on GPUs (GPGPU). CUDA is a software layer with a C-like programming interface that gives direct access to the GPU's virtual instruction set and parallel computational elements, for the execution of compute kernels. CUDA was first released in 2007 by Nvidia, and the vision of CUDA is not for graphics but for parallel computation. It enables GPUs to perform many parallelised floating points computations.  

Below is a short video from Nvidia introducing some basic of the basics of CUDA: 

<iframe width="640" height="480" src="https://www.youtube.com/embed/IzU4AVcMFys" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen=""></iframe> 
  
#### CUDA Scalable Parallel Architecture 

Before we move to more details about CUDA programming, we'll first look at the programming model of CUDA: 

![]()
_Figure 6:_ CUDA Parallel Thread Architecture 

###### _(1) Thread and Thread Block_

A CUDA program comprises of a host program, consisting of one or more sequential _threads_ running on a host, and one or more parallel kernels (a kernel acts as a function that runs on the device) suitable for execution on a parallel computing GPU. Only one kernel is executed at a time, and that kernel is executed on a set of lightweight parallel threads. For better resource allocation (avoid redundant computation, reduce bandwidth from shared memory), threads are grouped into thread blocks. A _thread block_ is a programming abstraction that represents a group of threads that can be executed serially or in parallel. 

###### _(2) Grid_ 

Multiple thread blocks are grouped to form a _grid_. Threads from different blocks in the same grid can coordinate using atomic operations on a global memory space shared by all threads. Sequentially dependent kernel grids can synchronise through global barriers and coordinate through global shared memory. Thread blocks implement coarse-grained scalable data parallelism and provide task parallelism when executing different kernels, while lightweight threads within each thread block implement fine-grained data parallelism and provide fine-grained thread-level parallelism when executing different paths. 

###### _(3) Scheduling of CUDA thread blocks_

The global work scheduler distributes CUDA thread blocks to SMs with available capacity, balancing load across a GPU, and running multiple kernel tasks in parallel if appropriate. The multithreaded SMs schedule and execute CUDA thread blocks and individual threads. Each SM can process multiple concurrent threads to hide long-latency loads from DRAM memory. Each thread block completes executing its kernel program and releases its SM resources before the work scheduler assigns a new thread block to that SM. A block is assigned to and executed on a single SM.  

### CUDA Software and Memory
     
#### CUDA Software Stack 

CUDA is implemented and deployed in multiple software layers. It consists of:  

  * The CUDA hardware driver  
  * The CUDA API and its runtime: The CUDA API is an extension of the C programming language that adds the ability to specify thread-level parallelism in C and also to specify GPU device-specific operations (like moving data between the CPU and the GPU). 
  * Mathematical libraries that have been optimised to run using CUDA.  

The CUDA software comes with the software driver, the CUDA toolkit (compiler, debugger, profiler), and the CUDA SDK (CUDA code samples).  

![]()
_Figure 7:_ CUDA Software Stack 

The CUDA Toolkit includes many sub-components. For example, for CUDA 11.6, the table below indicates the components: 

<table rules="all" border="1" frame="border" id="cuda-major-component-versions__table-cuda-toolkit-component-versions" summary="" cellspacing="0" cellpadding="4" style="border-collapse: collapse; margin-left: auto; margin-right: auto;"> 
  <caption> 
   Table 1. CUDA 11.6 Component Versions 
  </caption> 
  <thead align="left"> 
   <tr> 
    <th colspan="1" rowspan="1" align="center" valign="top" id="d117e134">Component Name</th> 
    <th colspan="1" rowspan="1" align="center" valign="top" id="d117e137">Version Information</th> 
    <th colspan="1" rowspan="1" align="center" valign="top" id="d117e140">Supported Architectures</th> 
   </tr> 
  </thead> 
  <tbody> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA C++ Core Compute Libraries</td> 
    <td colspan="1" rowspan="1" valign="top">11.6.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA Runtime (cudart)</td> 
    <td colspan="1" rowspan="1" valign="top">11.6.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">cuobjdump</td> 
    <td colspan="1" rowspan="1" valign="top">11.6.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUPTI</td> 
    <td colspan="1" rowspan="1" valign="top">11.6.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA cuxxfilt (demangler)</td> 
    <td colspan="1" rowspan="1" valign="top">11.6.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA Demo Suite</td> 
    <td colspan="1" rowspan="1" valign="top">11.6.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA GDB</td> 
    <td colspan="1" rowspan="1" valign="top">11.6.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA Memcheck</td> 
    <td colspan="1" rowspan="1" valign="top">11.6.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA Nsight</td> 
    <td colspan="1" rowspan="1" valign="top">11.6.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA NVCC</td> 
    <td colspan="1" rowspan="1" valign="top">11.6.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA nvdisasm</td> 
    <td colspan="1" rowspan="1" valign="top">11.6.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA NVML Headers</td> 
    <td colspan="1" rowspan="1" valign="top">11.6.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA nvprof</td> 
    <td colspan="1" rowspan="1" valign="top">11.6.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA nvprune</td> 
    <td colspan="1" rowspan="1" valign="top">11.6.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA NVRTC</td> 
    <td colspan="1" rowspan="1" valign="top">11.6.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA NVTX</td> 
    <td colspan="1" rowspan="1" valign="top">11.6.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA NVVP</td> 
    <td colspan="1" rowspan="1" valign="top">11.6.58</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA Samples</td> 
    <td colspan="1" rowspan="1" valign="top">11.6.101</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA Compute Sanitizer API</td> 
    <td colspan="1" rowspan="1" valign="top">11.6.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA cuBLAS</td> 
    <td colspan="1" rowspan="1" valign="top">11.8.1.74</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA cuFFT</td> 
    <td colspan="1" rowspan="1" valign="top">10.7.0.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA cuFile</td> 
    <td colspan="1" rowspan="1" valign="top">1.2.0.100</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA cuRAND</td> 
    <td colspan="1" rowspan="1" valign="top">10.2.9.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA cuSOLVER</td> 
    <td colspan="1" rowspan="1" valign="top">11.3.2.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA cuSPARSE</td> 
    <td colspan="1" rowspan="1" valign="top">11.7.1.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA NPP</td> 
    <td colspan="1" rowspan="1" valign="top">11.6.0.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">CUDA nvJPEG</td> 
    <td colspan="1" rowspan="1" valign="top">11.6.0.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">Nsight Compute</td> 
    <td colspan="1" rowspan="1" valign="top">2022.1.0.12</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64 (CLI only)</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">NVTX</td> 
    <td colspan="1" rowspan="1" valign="top">1.21018621</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">Nsight Systems</td> 
    <td colspan="1" rowspan="1" valign="top">2021.5.2.53</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64 (CLI only)</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">Nsight Visual Studio Edition (VSE)</td> 
    <td colspan="1" rowspan="1" valign="top">2022.1.0.21343</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64 (Windows)</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">nvidia_fs</td> 
    <td colspan="1" rowspan="1" valign="top">2.10.3</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">Visual Studio Integration</td> 
    <td colspan="1" rowspan="1" valign="top">11.6.55</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64 (Windows)</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">NVIDIA Linux Driver</td> 
    <td colspan="1" rowspan="1" valign="top">510.39.01</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64, POWER, Arm64</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top">NVIDIA Windows Driver</td> 
    <td colspan="1" rowspan="1" valign="top">511.23</td> 
    <td colspan="1" rowspan="1" valign="top">x86_64 (Windows)</td> 
   </tr> 
  </tbody> 
 </table> 
  
It is noted that the version of components differs version by version. In this unit, we will only cover a subset of these components, but if you are interested in any of them, you can find details on the <a href="https://docs.nvidia.com/cuda/" target="_blank" rel="noopener">CUDA Toolkit Documentation</a>. 

#### CUDA Memory Model 

Like other shared memory programming frameworks, CUDA has a memory model. The memory of CUDA is organised as the following diagram: 


![]()
_Figure 8:_ CUDA Memory Model 

Because of the nature of data allocation in shared memory, two concurrent threads in a warp can access different words in the same bank at the same time, causing a bank conflict that makes a GPU serialise the issued accesses to this bank. Since serialisation in a GPU is undesirable and clock-cycle costly, this access pattern should be avoided. 

The amount of memory that is available to the CUDA application is (in most cases) specific to the compute capability of the device. For each compute capability, the size restrictions of each type of memory (except global memory) is defined in the table below. The application programmer is encouraged to query the device properties in the application using the  cudaGetDeviceProperties() method. 
 
<table frame="box" border="2" style="border-collapse: collapse; width: 584.266px; margin-left: auto; margin-right: auto;"> 
  <caption> 
   Compute Capability 
  </caption> 
  <thead> 
   <tr> 
    <td>_Technical Specifications_</td> 
    <td>_1.0_</td> 
    <td>_1.1_</td> 
    <td>_1.2_</td> 
    <td>_1.3_</td> 
    <td>_2.0_</td> 
   </tr> 
  </thead> 
  <tbody> 
   <tr> 
    <td>Number of 32-bit registers per MP</td> 
    <td colspan="2">8 K</td> 
    <td colspan="2">16 K</td> 
    <td>32 K</td> 
   </tr> 
   <tr> 
    <td>Maximum amount of shared memory per MP</td> 
    <td colspan="4">16 KB</td> 
    <td>48 KB</td> 
   </tr> 
   <tr> 
    <td>Amount of local memory per thread</td> 
    <td colspan="4">16 KB</td> 
    <td>512 KB</td> 
   </tr> 
   <tr> 
    <td>Constant memory size</td> 
    <td colspan="5">64 KB</td> 
   </tr> 
  </tbody> 
 </table> 
  
  
 The following table summarises the different memory types and the properties of those types: 


 <table frame="box" border="2" style="border-collapse: collapse; width: 584.266px; margin-left: auto; margin-right: auto;"> 
  <caption> 
   Properties of Memory Types 
  </caption> 
  <thead> 
   <tr> 
    <td>_Memory_</td> 
    <td>_Located_</td> 
    <td>_Cached_</td> 
    <td>_Access_</td> 
    <td>_Scope_</td> 
    <td>_Lifetime_</td> 
   </tr> 
  </thead> 
  <tbody> 
   <tr> 
    <td>Register</td> 
    <td>cache</td> 
    <td>n/a</td> 
    <td>Host: None
Kernel: R/W</td> 
    <td>thread</td> 
    <td>thread</td> 
   </tr> 
   <tr> 
    <td>Local</td> 
    <td>device</td> 
    <td>1.x: No
2.x: Yes</td> 
    <td>Host: None
Kernel: R/W</td> 
    <td>thread</td> 
    <td>thread</td> 
   </tr> 
   <tr> 
    <td>Shared</td> 
    <td>cache</td> 
    <td>n/a</td> 
    <td>Host: None
Kernel: R/W</td> 
    <td>block</td> 
    <td>block</td> 
   </tr> 
   <tr> 
    <td>Global</td> 
    <td>device</td> 
    <td>1.x: No
2.x: Yes</td> 
    <td>Host: R/W
Kernel: R/W</td> 
    <td>application</td> 
    <td>application</td> 
   </tr> 
   <tr> 
    <td>Constant</td> 
    <td>device</td> 
    <td>Yes</td> 
    <td>Host: R/W
Kernel: R</td> 
    <td>application</td> 
    <td>application</td> 
   </tr> 
  </tbody> 
 </table> 

                                   
  
### CUDA Basic Usage

#### CUDA Operation Procedure 

The programming model of CUDA provides a SIMT (single instruction multiple threads) model. In CUDA, the CPU and the GPU have to be worked in a pre-defined sequence, formulating heterogeneous computing. Data has to be transferred from a CPU (i.e. host) to a GPU (i.e. device), usually over a PCIe bus, before the computation is offloaded, and the result transferred back to the main memory. A typical sequence of operations for a CUDA C program is: 

  1. Declare and allocate the host and device memory. 
  2. Initialize host data. 
  3. Transfer data from the host memory to the device memory. 
  4. Load GPU program and execute one or more kernels. Data is cached on-chip for performance. 
  5. Transfer results from the device to the host. 
 
In the following part, we will cover how to use the CUDA toolkit and write CUDA-accelerated programs. 
 
#### First CUDA Example 
 
CUDA uses C-like syntax and adds its own primitives and APIs on top of C. To understand the difference, we first look at the classic "hello, world" example written in C and in CUDA. 

_C:_ 

```c
void c_hello() { 
    printf("Hello World!\n"); 
} 

int main() { 
    c_hello(); 
    return 0; 
}
```

_CUDA:_ 

```c
__global__ void cuda_hello() { 
    printf("Hello World from GPU!\n"); 
} 

int main() { 
    cuda_hello<<<1,1>>>(); 
    cudaDeviceSynchronize(); 
    return 0; 
}
```

A CUDA program has two pieces: (1) host code on the CPU which interfaces with the GPU and (2) kernel code which runs on the GPU. In the CUDA version, the '__global__' specifier indicates a function that runs on the device (GPU). Such a function can be called through host code, e.g. the main() function in the example, and is also known as a "kernel". The "<<<...>>>" specifies its execution configuration, and in CUDA terminology, this is called "kernel launch". 

In its simplest format, it looks like: 

```c
kernel_routine<<<griddim, blockdim>>>(args);
``` 

where 

  * griddim is the number of instances of the kernel (the "grid" size)  
  * blockdim is the number of threads within each instance  
  * args is a limited number of arguments, usually mainly pointers to arrays in graphics memory, and some constants which get copied by value 

The more general form allows griddim and blockdim to be 2D or 3D to simplify application programs. 

#### Compiling CUDA Programs 

To compile a CUDA program, the 'nvcc' compile should be used. Just like gcc, nvcc can take parameters to, for example, change the output file name, or include libraries. Note that the source file extension is '.cu' for CUDA programs. Below is a table of input files that nvcc accepts: 

<table rules="all" border="1" frame="border" summary="" cellspacing="0" cellpadding="4"> 
  <thead align="left"> 
   <tr> 
    <th colspan="1" rowspan="1" valign="top" id="d117e399">Input File Prefix</th> 
    <th colspan="1" rowspan="1" valign="top" id="d117e402">Description</th> 
   </tr> 
  </thead> 
  <tbody> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top"><samp>.cu</samp></td> 
    <td colspan="1" rowspan="1" valign="top">CUDA source file, containing host code and device functions</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top"><samp>.c</samp></td> 
    <td colspan="1" rowspan="1" valign="top">C source file</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top"><samp>.cc</samp>, <samp>.cxx</samp>, <samp>.cpp</samp></td> 
    <td colspan="1" rowspan="1" valign="top">C++ source file</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top"><samp>.ptx</samp></td> 
    <td colspan="1" rowspan="1" valign="top">PTX intermediate assembly file (see Figure 9)</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top"><samp>.cubin</samp></td> 
    <td colspan="1" rowspan="1" valign="top">CUDA device code binary file (CUBIN) for a single GPU architecture (see Figure 9)</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top"><samp>.fatbin</samp></td> 
    <td colspan="1" rowspan="1" valign="top">CUDA fat binary file that may contain multiple PTX and CUBIN files (see Figure 9)</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top"><samp>.o</samp>, <samp>.obj</samp></td> 
    <td colspan="1" rowspan="1" valign="top">Object file</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top"><samp>.a</samp>, <samp>.lib</samp></td> 
    <td colspan="1" rowspan="1" valign="top">Library file</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top"><samp>.res</samp></td> 
    <td colspan="1" rowspan="1" valign="top">Resource file</td> 
   </tr> 
   <tr> 
    <td colspan="1" rowspan="1" valign="top"><samp>.so</samp></td> 
    <td colspan="1" rowspan="1" valign="top">Shared object file</td> 
   </tr> 
  </tbody> 
 </table> 
 
 
To compile the "hello, world" example, in a new terminal: 

```shell
$ nvcc hello.cu -o hello
```

Once the compilation is finished, the program can be run by: 

```shell
$ ./hello
``` 

CUDA compilation works as follows: the input program is preprocessed for device compilation and is compiled to a CUDA binary (<samp>cubin</samp>) and/or PTX intermediate code, which are placed in a fatbinary. The input program is preprocessed once again for host compilation and is synthesised to embed the fatbinary and transform CUDA specific C++ extensions into standard C++ constructs. Then the C++ host compiler compiles the synthesised host code with the embedded fatbinary into a host object. The exact steps that are followed to achieve this are displayed in the following diagram:

![]()
_Figure 9:_ CUDA compilation process -- from .cu to bin (source: Nvidia) 

The embedded fatbinary is inspected by the CUDA runtime system whenever the device code is launched by the host program to obtain an appropriate fatbinary image for the current GPU. 

### Vectorisation with CUDA
     
#### Vect Add example 

Consider the following 'vect_add.c' example, where two vectors of size N are added together (i.e. out[i] = a[i] + b[i]): 

```c
#define N 1024*256

void vector_add(float *out, float *a, float *b, int n) {
    for(int i = 0; i < n; i++){
        out[i] = a[i] + b[i];
    }
}

int main(){
    float *a, *b, *out; 

    // Allocate memory
    a   = (float*)malloc(sizeof(float) * N);
    b   = (float*)malloc(sizeof(float) * N);
    out = (float*)malloc(sizeof(float) * N);

    // Initialize array
    for(int i = 0; i < N; i++){
        a[i] = 1.0f; b[i] = 2.0f;
    }

    // Main function
    vector_add(out, a, b, N);
}
```

To vectorise this operation in CUDA, we first need to allocate memory on the GPU. This is because host and device memory are separate entities, and to run programs on the device, we need the data to be allocated on the device memory. CUDA provides some facilities which are similar to their C equivalents, which are: 

  * cudaMalloc(void **  devPtr, size_t  size) <---> malloc() 
  * cudaFree(void *  devPtr) <---> free() 
  * cudaMemcpy(void *  dst, const void *  src, size_t  count, enum cudaMemcpyKind  kind) <---> memcpy() 

For the memory copy function, depending on where the source and target data are, there are different values for the 'kind' argument. A full list is given below: 

<table cellpadding="0" cellspacing="2" border="1" style="border-collapse: collapse; height: 282px; border-color: #000000; border-style: solid;"> 
  <tbody> 
   <tr style="height: 47px;"> 
    <td style="height: 47px; width: 726.140625px;">_Kind___</td> 
    <td style="height: 47px; width: 1009.875px;">_Source and Destination_</td> 
   </tr> 
   <tr style="height: 47px;"> 
    <td valign="top" style="height: 47px; width: 726.140625px;">cudaMemcpyHostToHost</td> 
    <td style="height: 47px; width: 1009.875px;">Host -> Host</td> 
   </tr> 
   <tr style="height: 47px;"> 
    <td valign="top" style="height: 47px; width: 726.140625px;">cudaMemcpyHostToDevice</td> 
    <td style="height: 47px; width: 1009.875px;">Host -> Device</td> 
   </tr> 
   <tr style="height: 47px;"> 
    <td valign="top" style="height: 47px; width: 726.140625px;">cudaMemcpyDeviceToHost</td> 
    <td style="height: 47px; width: 1009.875px;">Device -> Host</td> 
   </tr> 
   <tr style="height: 47px;"> 
    <td valign="top" style="height: 47px; width: 726.140625px;">cudaMemcpyDeviceToDevice</td> 
    <td style="height: 47px; width: 1009.875px;">Device -> Device</td> 
   </tr> 
   <tr style="height: 47px;"> 
    <td valign="top" style="height: 47px; width: 726.140625px;">cudaMemcpyDefault</td> 
    <td style="height: 47px; width: 1009.875px;">Default based unified virtual address space</td> 
   </tr> 
  </tbody> 
 </table> 
  
We can now write a Kernel function with the decorator '__global__' to indicate that this is a device program: 

```c
__global__ void vector_add(float *out, float *a, float *b, int n) {
    for(int i = 0; i < n; i++){
        out[i] = a[i] + b[i];
    }
}
```

And then in the main() function, call the vector_add() kernel: 

```c
void main(){
    float *a, *b, *out;
    float *d_a, *d_b, *d_out;

    // Allocate host memory for a
    a = (float*)malloc(sizeof(float) * N);
    b = (float*)malloc(sizeof(float) * N);

    ...

    // Allocate device memory for a
    cudaMalloc((void**)&d_a, sizeof(float) * N);
    cudaMalloc((void**)&d_b, sizeof(float) * N);

    // Transfer data from host to device memory
    cudaMemcpy(d_a, a, sizeof(float) * N, cudaMemcpyHostToDevice);
    cudaMemcpy(d_b, b, sizeof(float) * N, cudaMemcpyHostToDevice);

    vector_add<<<1,1>>>(d_out, d_a, d_b, N);

    ...

    // Cleanup after kernel execution
    cudaFree(d_a);
    cudaFree(d_b);
    free(a);
    free(b);
}
```

Note some irrelevant code is omitted, for example, the initialisation of a and b.  

#### Profiling Performance 

NVIDIA provides a command-line profiler tool called 'nvprof', which gives more insight in the performance of CUDA applications. To profile our vector addition, use the following command: 

```shell
$ nvprof ./vector_add
```

An example output could look like this: 

```shell
==6326== Profiling application: ./vector_add
==6326== Profiling result:
Time(%)      Time     Calls       Avg       Min       Max  Name
 97.55%  1.42529s         1  1.42529s  1.42529s  1.42529s  vector_add(float*, float*, float*, int)
  1.39%  20.318ms         2  10.159ms  10.126ms  10.192ms  [CUDA memcpy HtoD]
  1.06%  15.549ms         1  15.549ms  15.549ms  15.549ms  [CUDA memcpy DtoH]
```

To get a more detailed trace, use the '--print-gpu-trace' flag. 

#### Kernel Execution Configuration 

Note that so far, we have not exploited the full power of GPU as we only apply <<1,1>> as the kernel execution configuration, which means we only use one GPU thread. CUDA organises threads into a group called a "_thread block_". Kernels can launch multiple thread blocks, organised into a "_grid_" structure. This is specified by the kernel execution configuration. 

The general syntax of kernel execution configuration is as follows: 

```c
<< M, T >>
``` 

where M is the grid number (i.e. number of thread blocks), and T is the number of parallel threads within each thread block (i.e. block size). Unlike OpenMP where the workload can be automatically assigned, CUDA does require some thought over how the workload is distributed (and thus the data manipulated) for each grid/thread. To do this, CUDA provides 5 built-in variables:  

  * gridDim denotes the dimension of grid, and blockDim denotes the dimension of block; their types are dim3.  
  * blockIdx and threadIdx identify the block index within the grid and thread index within the block respectively, and their types are uint3.  
  * warpSize is int type, and identifies the warp size in threads, and it should be 32 for all compute capabilities. 

To give it a try, we can change the configuration of the vector_add from <<1,1>> to <<1,256>>, i.e., one block with 256 threads: 

```c
vector_add <<< 1 , 256 >>> (d_out, d_a, d_b, N);
```

and in the Kernel function, we let the _k_-th thread handle the computation of k, k+stride, k+stride*2. The stride is 256 as we set T to 256. In this case, the 0th thread handles 0, 256, 512; the 1st thread handles 1, 257, 513; the 2nd thread handles 2, 258, 514, etc: 

```c
__global__ void vector_add(float *out, float *a, float *b, int n) {
    int index = threadIdx.x;
    int stride = blockDim.x;

    for(int i = index; i < n; i += stride){
        out[i] = a[i] + b[i];
    }
}
``` 

Similarly, we can also change the grid size M. With 256 threads per thread block, we need at least N/256 thread blocks to have a total of N threads. To assign a thread to a specific element, we need to know a unique index for each thread. Such an index can be computed as follow: 

```c
int tid = blockIdx.x * blockDim.x + threadIdx.x;
```

and finally, we can put everything together: 

```c
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <cuda.h>
#include <cuda_runtime.h>

#define N 1024*256
#define MAX_ERR 1e-6

__global__ void vector_add(float *out, float *a, float *b, int n) {
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    
    // Handling arbitrary vector size
    if (tid < n){
        out[tid] = a[tid] + b[tid];
    }
}

int main(){
    float *a, *b, *out;
    float *d_a, *d_b, *d_out; 

    // Allocate host memory
    a   = (float*)malloc(sizeof(float) * N);
    b   = (float*)malloc(sizeof(float) * N);
    out = (float*)malloc(sizeof(float) * N);

    // Initialize host arrays
    for(int i = 0; i < N; i++){
        a[i] = 1.0f;
        b[i] = 2.0f;
    }

    // Allocate device memory 
    cudaMalloc((void**)&d_a, sizeof(float) * N);
    cudaMalloc((void**)&d_b, sizeof(float) * N);
    cudaMalloc((void**)&d_out, sizeof(float) * N);

    // Transfer data from host to device memory
    cudaMemcpy(d_a, a, sizeof(float) * N, cudaMemcpyHostToDevice);
    cudaMemcpy(d_b, b, sizeof(float) * N, cudaMemcpyHostToDevice);


    // Executing kernel 
    int block_size = 256;
    int grid_size = N / block_size;
    vector_add<<<grid_size,block_size>>>(d_out, d_a, d_b, N);
    
    // Transfer data back to host memory
    cudaMemcpy(out, d_out, sizeof(float) * N, cudaMemcpyDeviceToHost);

    // Verification
    for(int i = 0; i < N; i++){
        assert(fabs(out[i] - a[i] - b[i]) < MAX_ERR);
    }

    printf("PASSED\n");

    // Deallocate device memory
    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_out);

    // Deallocate host memory
    free(a); 
    free(b); 
    free(out);
}
```

> _Exercise (in the lab):_ Can you do a performance comparison of vect_add with <<1,1>>, <<1,256>> and <<1024,256>>? How much speedup can you get?
 
### Advanced CUDA
     
#### Kernel with 2D Indexing 

The kernel configuration can be 2-dimensional (as well as 3-dimensional). To access the second dimension, use .y attribute (and .z for indexing the third dimension).  

An example is: 

```c
__global__ void kernel (int* a, int dimx, int dimy) { 
    int ix = blockIdx.x * blockDim.x + threadIdx.x; 
    int iy = blockIdx.y * blockDim.y + threadIdx.y; 
    int idx = iy * dimx + ix; 
    a[idx] = a[idx] + 1; 
}
```

and in your main: 

```c
dim3 grid, block; 
block.x = 4; 
block.y = 4; 
grid.x = dimx / block.x; 
grid.y = dimy / block.y; 

kernel<<<grid, block>>>(d_a, dimx, dimy);
``` 

#### Unified Memory 

From _Compute Capability 3.0+_ and _CUDA 6.0+_: 

  * Unified Memory creates a pool of managed memory that is shared between the CPU and GPU and accessible to both using a single pointer. 
  * The system automatically migrates data allocated in Unified Memory between host and device. 

```c
int main() { 
    float *data; 
    cudaMallocManaged (&data, dataSize * sizeOf(float)); 
    ... 
    cudaFree(data);
    ...
}
```
 
#### Tips for Performance Considerations 

Up to this point we have still only scratched the surface of CUDA. There are a lot of implementation details of how CUDA programs should be running on the GPU. To improve the performance, below are some tips to get you started: 

 * Kernel Launch Configuration: 
    * Launch enough threads per SM to hide latency
    * Launch enough threadblocks to load the GPU  
 * Global memory:  
    * Maximise throughput (the GPU has lots of bandwidth, use it effectively) 
    * Use shared memory when applicable (over 1 TB/s bandwidth) 
 * GPU-CPU interaction: 
    * Minimise CPU/GPU idling, maximise PCIe throughput 
* Use analysis/profiling when optimising 

Feel free to do your research. To help you get started, here is a short tutorial: <a href="https://events.prace-ri.eu/event/989/sessions/3077/attachments/1195/2032/cuda-efficiency.pdf" target="_blank" rel="noopener">GPU programming in CUDA: How to write efficient CUDA programs</a>. We will cover some of these concepts in the practical. 

# References and Further Reading
     
  * <a href="https://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html" target="_blank" rel="noopener">CUDA C++ Programming Guide</a>, Nvidia.  
  * <a href="https://docs.nvidia.com/cuda/pdf/CUDA_C_Programming_Guide.pdf" target="_blank" rel="noopener">CUDA C++ Programming Guide (pdf version)</a>, Nvidia. 
  * <a href="https://www.nvidia.com/content/PDF/fermi_white_papers/P.Glaskowsky_NVIDIA's_Fermi-The_First_Complete_GPU_Architecture.pdf" target="_blank" rel="noopener">NVIDIA’s Fermi: The First Complete GPU Computing Architecture</a>, Nvidia. 
  * <a href="https://people.maths.ox.ac.uk/gilesm/cuda/lecs/lecs.pdf" target="_blank" rel="noopener">An Introduction to CUDA</a>, Mike Giles, Oxford University. 
  * <a href="https://www.tutorialspoint.com/cuda/index.htm" target="_blank" rel="noopener">Learn CUDA - CUDA Tutorials</a>, Tutorials Point. 
