---
title: 4. Programming On-core Parallelism 
date: 2022-07-21
category: hipc
layout: post
---

# Overview

In the fourth HIPC practical you're going to be exploring vectorisation in-depth. This is something the compiler will often do for you (and will typically do it better, or at least in much less time). The important thing in this practical is not to get the most optimised vectorised source, but just to understand a little more of what the compiler is doing when it vectorises code.

This practical is based around two exercises, the first is the spheres application from the last practical, and the second is a simple N-body simulation.

Again, for this practical we'll start with disabling compiler optimisations, so make sure you keep `-O0` throughout the following exercises, unless otherwise stated. 

> # Exercise 1
> 
> The code for calculating the number of intersections is a good target for vectorisation. 
> 
> Try rewriting the spheres application using vector intrinsics. Refer to the [Intel Intrinsics Guide](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html) for help. 
> 
> You will need to include the `immintrin.h` header file, and use the `-mavx` or `-mavx2` command line switch to enable the compiler to generate instructions for AVX and AVX2 instruction sets. 
>
> **Note:** On the lab machines you can almost certainly use AVX/AVX2 intrinsics. On your own machine, you may have to check what instruction sets you have available. You can do this by reviewing the output of `/proc/cpuinfo` (on Linux). e.g. 
> 
> ```
> $ cat /proc/cpuinfo
> processor         : 0
> vendor_id         : GenuineIntel
> cpu family        : 6
> model             : 158
> model name        : Intel(R) Core(TM) i7-8700K CPU @ 3.70GHz
> stepping          : 10
> microcode         : 0xea
> cpu MHz           : 800.689
> cache size        : 12288 KB
> physical id       : 0
> siblings          : 12
> core id           : 0
> cpu cores         : 6
> apicid            : 0
> initial apicid    : 0
> fpu               : yes
> fpu_exception     : yes
> cpuid level       : 22
> wp                : yes
> flags             : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi 
> mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc art arch_perfmon pebs bts rep_good 
> nopl xtopology nonstop_tsc cpuid aperfmperf pni pclmulqdq dtes64 monitor ds_cpl vmx smx est tm2 ssse3 sdbg fma 
> cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm 
> 3dnowprefetch cpuid_fault invpcid_single pti ssbd ibrs ibpb stibp tpr_shadow vnmi flexpriority ept vpid ept_ad 
> fsgsbase tsc_adjust bmi1 hle avx2 smep bmi2 erms invpcid rtm mpx rdseed adx smap clflushopt intel_pt xsaveopt 
> xsavec xgetbv1 xsaves dtherm ida arat pln pts hwp hwp_notify hwp_act_window hwp_epp md_clear flush_l1d
> bugs              : cpu_meltdown spectre_v1 spectre_v2 spec_store_bypass l1tf mds swapgs taa itlb_multihit srbds
> bogomips          : 7399.70
> clflush size      : 64
> cache_alignment   : 64
> address sizes     : 39 bits physical, 48 bits virtual
> power management  :
> ...
> ```
>  
> **Note:** "`avx`" and "`avx2`" are both listed under the "`flags`" category. Alternatively, you could Google the CPU model. <br/><br/>
{: .block-danger }

> # Exercise 2
>
> Try compiling the original application and your optimised code with `-O3`. Which is faster and why? Review the optimisation reports. <br/><br/>
{: .block-danger }

> # Exercise 3
>
> Attached File: [`nbody.c`](../../assets/practical-3/nbody.c)
>
> Here is a simple [n-body code](https://en.wikipedia.org/wiki/N-body_simulation). In the code there are a series of particles in 3D space, where each particle has position in space (x, y, z), a velocity vector (vx, vy, vz), and a mass (m). The code calculates the force felt on each particle by all other particles (as an acceleration) and then updates their velocity. The code then updates each particles position based on the velocity and the time step size. 
> 
> At the end of the computation, the code calculates a simple checksum that can be used to ensure our simulation is correct. 
>  
> ```c
> #include <stdio.h>
> #include <stdlib.h>
> #include <math.h>
> #include <immintrin.h>
> 
> #include <sys/time.h>
> #include <sys/times.h>
> #include <sys/resource.h>
> 
> // Simulation parameters.
> float eps = 0.00125f;
> float dmp = 0.995f;
> float dt = 0.001f;
> 
> /**
>  * Main function.
>  */
> 
> int main(int argc, char *argv[]) {
>     int N = atoi(argv[1]);
>     int steps = atoi(argv[2]);
> 
>     // Allocate arrays, aligned to 16 bytes.
>     float *x = (float *) malloc(N * sizeof(float));
>     float *y = (float *) malloc(N * sizeof(float));
>     float *z = (float *) malloc(N * sizeof(float));
>     float *vx = (float *) malloc(N * sizeof(float));
>     float *vy = (float *) malloc(N * sizeof(float));
>     float *vz = (float *) malloc(N * sizeof(float));
>     float *m = (float *) malloc(N * sizeof(float));
> 
>     // Initialise array contents.
>     srand(42);
>     for (int i = 0; i < N; i++) {
>         x[i] = ((rand() / (float)RAND_MAX) * 2 - 1);
>         y[i] = ((rand() / (float)RAND_MAX) * 2 - 1);
>         z[i] = ((rand() / (float)RAND_MAX) * 2 - 1);
>         vx[i] = ((rand() / (float)RAND_MAX) * 2 - 1);
>         vy[i] = ((rand() / (float)RAND_MAX) * 2 - 1);
>         vz[i] = ((rand() / (float)RAND_MAX) * 2 - 1);
>         m[i] = (rand() / (float)RAND_MAX);
>     }
> 
>     // Current timestep.
>     int t = 0;
> 
>     while (t < steps) {
>         for (int i = 0; i < N; i++) {
>             // start with zero acceleration for each particle
>             float ax = 0.0f;
>             float ay = 0.0f;
>             float az = 0.0f;
> 
>             // calculate the acceleration on particle i from all other particles
>             for (int j = 0; j < N; j++) {
>                 float rx = x[j] - x[i];
>                 float ry = y[j] - y[i];
>                 float rz = z[j] - z[i];
> 
>                 float r2 = rx * rx + ry * ry + rz * rz + eps;
> 
>                 float r2inv = 1.0f / sqrt(r2);
> 
>                 float r6inv = r2inv * r2inv * r2inv;
> 
>                 float s = m[j] * r6inv;
> 
>                 ax += s * rx;
>                 ay += s * ry;
>                 az += s * rz;
>             }
> 
>             // update the velocity based on calculated acceperation
>             vx[i] += dmp * (dt * ax);
>             vy[i] += dmp * (dt * ay);
>             vz[i] += dmp * (dt * az);
>         }
> 
>         // update the position of each particle
>         // reflect boundary particles
>         for (int i = 0; i < N; i++) {
>             x[i] += dt * vx[i];
>             y[i] += dt * vy[i];
>             z[i] += dt * vz[i];
>             if (x[i] >= 1.0f || x[i] <= -1.0f) vx[i] *= -1.0f;
>             if (y[i] >= 1.0f || y[i] <= -1.0f) vy[i] *= -1.0f;
>             if (z[i] >= 1.0f || z[i] <= -1.0f) vz[i] *= -1.0f;
>         }
>         t++;
>     }
> 
>     float phi = 0.0f;
>     for (int i = 0; i < N; i++) {
>         for (int j = 0; j < N; j++) {
>             float rx = x[j] - x[i];
>             float ry = y[j] - y[i];
>             float rz = z[j] - z[i];
>             float r2 = rx*rx + ry*ry + rz*rz + eps;
>             float r2inv = 1.0 / sqrt(r2);
>             float r6inv = r2inv * r2inv * r2inv;
>             phi += m[j] * r6inv;
>         }
>     }
> 
>     printf(" Answer = %f\n", phi);
> 
>     // Tidy up.
>     free(x);
>     free(y);
>     free(z);
>     free(vx);
>     free(vy);
>     free(vz);
>     free(m);
> }
> ```
>  
> Add timers to the application and then optimise it as best as you can. Bonus marks if you are able to make use of vector intrinsics in the main loop body.  <br/><br/>
{: .block-danger }

> # Exercise 4
>
> Again, use Intel Advisor to analyse your solutions and the previous implementations. <br/><br/>
{: .block-danger }
