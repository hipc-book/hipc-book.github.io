---
title: 6. Advanced MPI Programming
date: 2022-07-21
category: hipc
layout: post
---

> **This practical is a work-in-progress**
{: .block-danger }

# Overview

# Profiling Your Applications in Parallel

Profiling applications that are running across a supercomputer can be _**hard**_. 

The easiest approach is to instrument your code with timers as before. But you should be aware that each compute node may have a different time (while NTP tends to keep nodes in check timewise, there's still a possibility of drift and even a few milliseconds of drift can affect our results). 

For this reason you should note the following things: 

* You should collect timing information from each rank separately (combining timers will likely skew results);
* _**If**_ you're aggregating times, you will need to synchronise your application (perhaps an `MPI_Barrier()` followed by a call to `MPI_Wtime()`) to calculate each ranks time offset before you do anything else;
* Because many MPI calls will block until completion, compute imbalance may skew timers. If one rank is significantly faster than another, it will still potentially wait on a blocking call.

## Profiling Tools

Besides manual instrumentation, there are a number of performance profiling tools available (some of which are installed on Viking!). 

You can find a list of the tools available on Viking with: 
 
```
$ module avail perf
```

The tools to pay closest attention to are: **Intel Advisor**, **Scalasca** and **PAPI**.  

Intel Advisor will provide advice on how to optimise your application on a single node (i.e. it doesn't profile MPI applications). 

Scalasca is a tracing tool designed to record and analyse the performance of parallel applications running on a cluster. You can find more information on how to use Scalasca here: [Scalasca: Getting Started](http://apps.fz-juelich.de/scalasca/releases/scalasca/2.6/docs/manual/start.html) 

PAPI counters are available on Viking for you to instrument your code using CPU performance counters. More information on PAPI can be found in Unit 3. 

   
   
     
