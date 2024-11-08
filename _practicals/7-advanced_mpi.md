---
title: 7. Advanced MPI Programming
date: 2022-07-21
category: hipc
layout: post
---

# Overview

In the practical this week we're going to build on the work we did in the previous practical, looking at MPI. If you've not finished the last practical, you should focus on that before moving on to this practical.

We're going to look at two of the exercises you did in the previous practical and we're going to apply some of the MPI functionality covered in Unit 7. Specifically, we're going to look at non-blocking communications and MPI-IO. We'll finish this practical with a look at profiling and debugging parallel applications.

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

# MPI-IO

In Units 6 and 7, MPI-IO has been covered showing how processes can work together to write a single output file in parallel. The next exercise is to make use of this functionality in one of the codes you wrote in the last practical.

> # Exercise 2
> 
> The simple steady-state heat equation application you encountered in the last practical (Exercise 4) produced a CSV file at the end of execution.
>
> When parallelising the application, it was suggested that you disable I/O at this time and just focus on the parallelisation. Assuming you have successfully parallelised the application, we can now revisit this exercise and add our I/O routines back in.
>
> Add MPI-IO to your application to produce an equivalent CSV file in parallel (i.e. without gathering the grid back to a single rank). Once you have achieved this, check your output file against the single-threaded implementation.
>
> **Hint**: To get the output in a readable format you might need to use a line buffer (i.e. a `char` array) that is allocated to be big enough to hold a large number of CSV values, and you may need to make use of functionality such as `sprintf` to generate a readable string. Alternatively, you could write out your output using the `MPI_DOUBLE` datatype, but this will be a binary file.<br/><br/>
{: .block-danger }


# Profiling and Debugging Your Applications in Parallel

Profiling and debugging applications that are running across a supercomputer can be _**hard**_. 

## Profiling Applications

The easiest approach to profiling is to instrument your code with timers as before. But you should be aware that each compute node may have a different time (while NTP tends to keep nodes in check time wise, there's still a possibility of drift and even a few milliseconds of drift can affect our results). 

For this reason, you should note the following things: 

* You should collect timing information from each rank separately (combining timers will likely skew results);
* _**If**_ you're aggregating times, you will need to synchronise your application (perhaps an `MPI_Barrier()` followed by a call to `MPI_Wtime()`) to calculate each ranks time offset before you do anything else;
* Because many MPI calls will block until completion, compute imbalance may skew timers. If one rank is significantly faster than another, it will still potentially wait on a blocking call.

### Profiling Tools

Besides manual instrumentation, there are a number of performance profiling tools available (some of which are installed on Viking!). 

The tools to pay closest attention to are: **Scalasca** and **PAPI** (and **Intel Advisor** if it is installed!).  

Scalasca is a tracing tool designed to record and analyse the performance of parallel applications running on a cluster. You can find more information on how to use Scalasca here: [Scalasca: Getting Started](http://apps.fz-juelich.de/scalasca/releases/scalasca/2.6/docs/manual/start.html) 

PAPI counters are available on Viking for you to instrument your code using CPU performance counters. More information on PAPI can be found in Unit 3. 

Intel Advisor will provide advice on how to optimise your application on a single node (i.e. it doesn't profile MPI applications). 

## Debugging Applications

Debugging MPI applications is difficult! Essentially, MPI ranks are entirely separate applications that just happen to be running the same code, and exchanging messages.

There are however some rather _hacky_ solutions to debugging applications in parallel.

It is perhaps easier to debug on a local machine with MPI installed, rather than debugging on a remote system in a batch job. One solution, suggested by OpenMPI, is to run multiple instances of a terminal emulator (e.g. `xterm`) and run the MPI applications in that shell (inside `gdb`). For example,

```bash
$ mpirun -np 4 xterm -e gdb ./my_mpi_application
```

This will spawn 4 `xterm` windows, and each of these windows will start your MPI application inside a `gdb` environment. If you start each of these `gdb` sessions with the `run` command, everything should work (vaguely speaking!).

Another solution is to simply write a command such that should your application fail in a non-interactive `gdb` session, it runs a backtrace (`bt`) to help you find the problematic area.

For example,

```bash
$ mpirun -n 2 gdb -ex "set width 1000" -ex "thread apply all bt" -ex run -ex bt -ex "set confirm off" -ex quit --args ./my_application [PROGRAM ARGUMENTS]
```

This command runs multiple copies of `gdb`, passing a number of commands to `gdb` such that should a segmentation fault occur, it will run a backtrace. As an example, it might produce something like the following:

```
$ mpirun -n 2 gdb -ex "set width 1000" -ex "thread apply all bt" -ex run -ex bt -ex "set confirm off" -ex quit --args ./mpi-test
GNU gdb (GDB) Red Hat Enterprise Linux 8.2-19.el8
Copyright (C) 2018 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "x86_64-redhat-linux-gnu".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<http://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
GNU gdb (GDB) Red Hat Enterprise Linux 8.2-19.el8
Copyright (C) 2018 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "x86_64-redhat-linux-gnu".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<http://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from ./mpi-test...Reading symbols from ./mpi-test...done.
done.
Starting program: /users/xxx999/MPI/mpi-test 
Starting program: /users/xxx999/MPI/mpi-test 
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib64/libthread_db.so.1".
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib64/libthread_db.so.1".
[New Thread 0x7ffff6b37700 (LWP 1002878)]
[New Thread 0x7ffff6b37700 (LWP 1002877)]
[New Thread 0x7ffff5f1e700 (LWP 1002879)]
[New Thread 0x7ffff5f1e700 (LWP 1002880)]
Hello from rank 0 of 2
Hello from rank 1 of 2

Thread 1 "mpi-test" received signal SIGSEGV, Segmentation fault.
0x0000000000400bc4 in main (argc=1, argv=0x7fffffff6a58) at mpi-test.c:33
33	            buffer[i] = i;
#0  0x0000000000400bc4 in main (argc=1, argv=0x7fffffff6a58) at mpi-test.c:33
^CException ignored in: <gdb.GdbOutputFile object at 0x7f384bb8ea90>
Traceback (most recent call last):
  File "/usr/share/gdb/python/gdb/__init__.py", line 43, in flush
    def flush(self):
```

From this, we can identify that the error in the application occurred in the file `mpi-test.c`, on line 33. It is likely that this line is erroring because we're using memory that had not been appropriately allocated.

Ultimately, debugging might be easier if you just liberally sprinkle `printf` statements everywhere, but it is useful to have many tools in your toolbox!

> # Exercise 3
> 
> Attempt one of the above solutions to debugging. Perhaps introduce a minor bug into one of your MPI exercises, and see if you can get a debugger to find the invented issue. <br/><br/>
{: .block-danger }
     
> **Further Reading**
>
> * [FAQ: Debugging applications in parallel](https://www.open-mpi.org/faq/?category=debugging#serial-debuggers), OpenMPI
> * [GDB with MPI](https://github.com/jeffhammond/HPCInfo/blob/master/docs/Debugging.md), Jeff Hammond
{: .block-tip }

