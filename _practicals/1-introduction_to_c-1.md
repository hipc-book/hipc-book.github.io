---
title: 1. Introduction to C (part 1)
date: 2022-07-21
category: hipc
layout: post
---

# Summary
     
This practical session should give you some familiarity with many of the core features of the C programming language and how to perform some simple debugging. 

Following this session you should be familiar with: 
 
* Writing basic C programs with input and output provided by `scanf` and `printf` 
* How to write simple functions in C 
* The basics of manipulating pointers and allocating dynamically sized arrays 
* A basic understanding of compiling multi-file projects, and using simple "makefiles" 
* How to use GDB to find errors in C programs 


# Getting Started

The aim of the first two labs is to expose you to the C programming language that will be used throughout this module. 

You should type out the code samples below, compile them and make sure you know exactly how each one works. There are exercises throughout that you should attempt. If you are struggling with any exercise please let us know. 
 
## Setup

Almost every HPC system in use today uses the Linux operating system (or a Unix variant). For this reason, it is expected that you will complete these practicals and the coursework on a Unix/Linux-based system. If you are unfamiliar with the Linux command line, there are many guides available (e.g. [The Linux command line for beginners](https://ubuntu.com/tutorials/command-line-for-beginners#1-overview)). If you are currently using a Windows lab machine, you should consider rebooting into Linux. If you are using a machine without Linux installed, perhaps consider SSHing to a Linux machine (e.g. the **csteach** machines). You may complete many of the exercises successfully on a MacOS system, but will likely find differences between the commands in this lab session and those required on a Mac. 

You can use any editor of your choice to write your codes. Some may prefer using advanced IDEs like VScode while others might prefer terminal based editors such as `vim` or `nano`. Whatever you use, you must learn how to use the editor/IDE by yourself. 

## Getting Started With C

We will begin these labs with the traditional "Hello, world!" program. Remember -- C is not an object-oriented language, meaning there are no classes or objects to worry about. Much like Java, C has a limited set of language features out of the box. Additional functionality is packaged into libraries which must then be included. This keeps the core language simple and is conceptually similar Java's `import` mechanism. To perform I/O to the terminal, we use the standard I/O library ([`stdio.h`](http://linux.die.net/man/3/stdio)). This contains the [`printf()`](http://linux.die.net/man/3/printf) function which is used to print a formatted string to the screen.  
 
**Note:** comments in C follow the same conventions as Java. `//` indicates a single line comment, `/* */` is a block comment. 


```c
#include <stdio.h> // Includes the stdio.h library 

int main(int argc, char *argv[]) { // main method is called when program is run    
   printf("Hello, world!\n");      // Use the printf function from stdio.h to print a string to the terminal    
   return 0;                       // return code zero means no problems occurred 
}
```

The `printf()` function prints a format string to standard out. Format strings can contain special control characters and variable placeholders in addition to regular text. In the example above we see `'\n'`, the newline control character. Save this code to a file called `helloworld.c` and then we can compile it. To compile the application we can use the C compiler from the GNU Compiler Collection (GCC) which should already be installed on the lab machines. To compile your program and create an executable file called "helloworld", run the following command in the terminal (do not forget to change to your working directory before running the command):
 
```
$ gcc -o helloworld helloworld.c
```

Assuming you were in the right directory and copied this code correctly this command will output an application named `helloworld` (as specified by the `-o` flag). We run this like any other Linux binary: 
 
```
$ gcc -o helloworld helloworld.c
$ ./helloworld
Hello, world!
$
```
 
You'll find that, for the most part, C and Java have the same semantics. You will already be familiar with C's flow control statements (loops and conditionals) as they were inherited by Java. One noticeable difference is that in C (prior to the C99 standard) variables must be declared outside of `for` loop conditions, otherwise they will result in a compiler error. Compare these two applications (in C and Java), paying special attention to where the variable controlling loop iterations, `i`, is defined. 

**Java** 
 
```java
class Sum {    
    public static void main(String args[]) {      
        int sum=0;
        for (int i=0; i < 10; i++) {          
            sum = sum + i;      
        }
        System.out.println("The sum is: " + sum);   
    }
}
```
 
 **C** 
 
```c
#include <stdio.h>

int main(int argc, char *argv[]) {    
    int sum = 0;
    int i;
    for (i = 0; i < 10; i++) {
        sum = sum + i;    
    }
    printf("The sum is: %d\n", sum);   
    return 0; 
}
```
 
**Note**: The format string provided to the `printf()` function is very different to Java's println function. 

# Functions

In the C language, each program consists of at least one function. Every C program must have a `main()` function, where the execution of the program begins. There can be additional functions (defined by the user) which make the program more modular and therefore easy to debug. Consider the following example: 
 
```c
#include <stdio.h>


int factorial(int m) {  // computes the factorial of a positive integer m
    int i, prod = 1;

    for (i = 2; i <= m; i++) {
        prod = prod * i;
    }
    return prod;
}

int main() {
    int n;

    printf("Enter a positive integer:");
    scanf("%d", &n); // scanf() scans the integer entered by the user

    printf("The factoral of %d is %d\n", n, factorial(n));
    return 0;
}
```

In this example, the `factorial()` function computes the factorial of a positive integer entered by the user. The first line of each C function should define the output and input(s) of the function. In this case, the line 
 
```c
int factorial(int m)
```
 
specifies that the function `factorial()` takes one integer input and returns an integer. A C function cannot return multiple values. If you want a C function to return multiple values, use pointers (more on those later) or custom types (more on those later, too). 

Note the use of the `scanf()` function in the above code. It scans input according to a format specifier (`"%d"` in this case). To know more about the function `scanf()` type 
 
```
$ man scanf
```
 
in the terminal and it will show you the `man` page (short for manual page) for the `scanf()` function. In general, if you would like to pull up the man page of any library function, type the following in the terminal 
 
```
$ man <function_name>
```
 
 
where `<function_name>` specifies the name of the function you want to search for. Try pulling up the manual page for the `man` function itself! 

> # Exercise 1
>
> Write a C program that asks the user to enter an integer, `n`, and then prints the primes between 2 and `n`. The program should use a function to determine if a given integer is a prime. <br/><br/>
{: .block-danger }

# Pointers

There are two fundamental types of variable in C: the primitive and the pointer. A pointer is something that points to a space in memory. In Java, pointers are hidden from the user and instead we talk about objects and primitives (although under the covers Java's "objects" are implemented as pointers to data in memory). In C we can manipulate memory in a much more powerful (and more dangerous!) manner. Pointers are identified using an asterisk. For example: 
 
```c
int a;   // An integer primitive, holds a whole number from -2,147,483,648 to 2,147,483,647 on most machines
int *b;  // An integer pointer, holds an address of an integer variable.
```
 
This declares a pointer (`b`) that will point to an integer in memory. Initially `b` won't be pointing at a valid integer but we can make it point to one. Consider the following:
 
```c
#include <stdio.h>

int main(int argc, char *argv[]) {
   int *a_pointer;
   int a_value = 5;
   int x[] = { 10, 1, 2, 5, -3 };

   a_pointer = &a_value;

   printf("The value of a_value is %d\n", a_value); // print the value of the variable a_value
   printf("The value of the pointer is %p,\n", a_pointer); // prints the address of the variable a_value
   printf("The value pointed to by the pointer is %d,\n", *a_pointer); // prints the value of the variable a_value

   *a_pointer = 10;

   printf("The value of a_value is now %d\n", a_value); // the value of a_value will be changed
   printf("The start address of the array x is %p\n", &x); // prints the base address of the array x
   printf("The address of the first element is %p\n", &x[0]); // also prints the same

   int i;
   for (i = 0; i < 5; i++) {
      printf("Value stored in address %p is %d\n", (x+i), x[i]);  // shows that array elements are stored in contiguous locations
   }

   printf("Size of each integer is %lu bytes\n", sizeof(int));  // the address of each location in array x differs by this amount from its previous location
   return 0;
}
```

**Note**: integers are formatted using `%d` in a `printf` statement; pointer memory addresses are formatted using `%p`. `%lu` is used to indicate a _long_, _unsigned integer_ (i.e. a 64-bit integer without a sign bit).

The `&` de-references a particular variable to its address, and so `&a_value` gets the address in memory where `a_value` is stored and stores it in `a_pointer`. This means that `a_pointer` is now 'pointing to' `a_value`.  

If we ever want to get the value being pointed to we de-reference the pointer using `*`. The statement `*a_pointer = 10` in the above code stores the value 10 in the address pointed by the pointer `a_pointer` which in this case is the address of `a_value`. Therefore, `a_value` gets updated to 10.  

Next, we show how addresses are organised within an array. The base address of the array `x` can be accessed either using `x` or `&x[0]`. Similarly the addresses of the other elements of the array can be accessed using `x+i` or `&x[i]`. Observe the outputs produced by the the statements within the loop. What can you conclude from them? 

For historical reasons the position of the `*` is not fixed when declaring a pointer variable. The following three lines are all equivalent. You should pick the style you like best and stick to it. 
 
```c
int* a;

int * a;

int *a;
```
 
While they may seem quite confusing, pointers are an incredibly powerful feature of C and are vital when writing efficient code. 

> # Exercise 2
>
> Given the code below, implement the `swap` function and call it correctly from the `main` function to swap the two integers (so that `a` becomes `b`, and `b` becomes `a`). 
>
> **Note**: even in this simple example we are beginning to see the power of C -- it is impossible to write an equivalent function to exchange primitives in Java. 
>  
> ```c
> #include <stdio.h>
> 
> // write the code for the function swap() that would swap the values of two integers
> 
> int main(int argc, char *argv[]) {
>    int val_a = 50;
>    int val_b = 20;
> 
>    // call the swap() function correctly, swap() should swap the values stored in val_a and val_b so
>    // that the swapped values of val_a and val_b are printed in the next two lines
> 
>    printf("val_a is %d (should be 20)\n", val_a);
>    printf("val_b is %d (should be 50)\n", val_b);
> 
>   return 0;
> }
> ```
> <br/>
{: .block-danger }

> # Exercise 3
>
> Complete the C program below by writing the code for the `sort()` function. You can use any sorting algorithm of your choice. 
>  
> ```c
> #include <stdio.h>
> 
> // write the code for the sort() function that sorts an integer array in ascending order
> 
> int main() {
>    int x[]= { 4, 1, 4, 3, 10, 5 };
>    int i;
> 
>    sort(x, 6); // sort() function sorts the array x in ascending order
> 
>    printf("The sorted array is as follows:\n");
> 
>    for (i=0; i < 6; i++){
>       printf("%d ", x[i]);
>    }
> 
>    printf("\n");
>    return 0;
> }
> ```
> <br/>
{: .block-danger }

# Memory

Pointers are at their most useful and powerful when you are dealing with memory. The standard library (`stdlib.h`) contains 3 useful functions for memory management: [`malloc()`](http://linux.die.net/man/3/malloc), [`calloc()`](http://linux.die.net/man/3/calloc), and [`realloc()`](http://linux.die.net/man/3/realloc). 
 
```c
void * malloc(size_t size);
```

`malloc()` takes a single argument, the size of the memory to allocate (in bytes) and returns the address of our newly allocated memory as a `void` pointer. Void pointers are simply pointers which may point to anything.  

Typically we want to allocate memory to store multiple instances of a specific datatype each of which is larger than 1 byte. We use the `sizeof()` function to get the size of a datatype and multiply it by the number of elements we intend to store. We then cast the pointer to the correct type. For example, to store 10 integers we do the following: 
 
```c
int *my_array = (int *) malloc(sizeof(int) * 10); // Be careful with the different meanings of '*' - here we see a pointer and multiply.
```
 
**Note**: We use `sizeof()` rather than hardcoding the size of an integer because in C, unlike Java, the size of an integer can vary between different architectures. This is because while Java runs on a standardised virtual machine, C has to run on real hardware which may have different word sizes. 

We can get the information out of this array in two ways: (1) like a normal array using square brackets; or, (2) by manipulating the pointer. Method 1 is usually the correct way to do things, as pointer manipulation is extremely error prone. The following code illustrates both methods: 
 
```c
int *my_array = (int *) malloc(sizeof(int) * 10);

my_array[0] = 1;
my_array[1] = 2;
...
my_array[9] = 10;

*my_array = 1;
*(my_array+1) = 2;
...
*(my_array+9) = 10;
```

**Note**: C does no range checking -- it will not prevent you reading or writing beyond the end of an array. This can lead to serious bugs (more on this later). 

These methods also work when getting values out of the array. You can also manipulate a pointer using notation such as `++`, but this will change the value of the pointer itself (meaning it will point to the second element) and when this is done, you may lose the ability to _release_ the memory later. 
 
```c
void *calloc(size_t nmemb, size_t size);
```

When memory is allocated using `malloc()`, there is no guarantee that the memory will be set to zeros. The `calloc()` function therefore allocates memory and also clears its contents (hence the name `calloc`). It takes two arguments, where the first is the number of elements to allocate space for and the second is the size of each element. So to allocate space for 10 integers (as we did with `malloc`), but to also set them all to zero:
 
```c
int *my_array = (int *) calloc(10, sizeof(int));
```
 
If you've already allocated some memory but find you actually need more memory, there is a function that will either extend the size of your allocation, or allocate a new bigger block elsewhere in memory and copy the old data across. This is called `realloc()` (since it re-allocates memory).
 
```c
void *realloc(void *ptr, size_t size);
```
 
The `realloc()` function does not zero data and requires 2 arguments to be provided to it. First, it requires the original pointer, and second it requires a size (in bytes) much like the `malloc()` function. For example, to increase `my_array` to 20 integers:

```c
int *tmp_ptr = (int *) realloc(my_array, sizeof(int) * 20);

if (tmp_ptr != NULL) my_array = tmp_ptr;
else perror("Failed to reallocate memory.");
```
 
You should check the return values of all `..alloc()` functions to ensure a pointer has been assigned. If an error occurs allocating memory, the functions all return the special `NULL` value. It is especially important to do this for the `realloc()` function. If you call `realloc` like so:

```c
my_array = (int*) realloc(my_array, sizeof(int) * 20);
```

In the case where the `realloc()` fails, you will lose the data that was originally in `my_array`, and so it is vital that you assign its result to a temporary pointer first.
 
The final piece of the memory allocation puzzle is that in C there is no garbage collector to clear up after you. Memory must be manually allocated and then deallocated when it is no longer needed. This is done with the [`free()`](http://linux.die.net/man/3/free) function. 
 
```c
void free(void *ptr);
```
 
The `free()` function returns no value and simply releases the memory in use by a pointer. After `free()` has been called the memory at `*ptr` will be released. The value of `ptr` will not change, however, you may wish to explicitly set `ptr` to `NULL` to prevent any invalid memory accesses. 

> # Exercise 4
>   
> Write a C program that would sort an array of any size entered by a user. The program should first ask the user the size of the array they want to sort. Then the program should ask the user to enter the elements of the array. Once the whole array is entered by the user, the program should output the sorted array.  
> 
> You should use a dynamic array to store the integers entered by the user. You can use the `scanf()` function to read the numbers input by the user.<br/><br/>
{: .block-danger } 

# Multiple File Projects

Often when working on large software projects it is convenient to break the project down into multiple source files. It makes programming and debugging much easier. In this section, we shall learn how to build an executable file from multiple source files written in C. 

Write the following code and save it as a .c file, say `main.c`: 
 
```c
#include <stdio.h>

int sum(int);
int sum_squares(int);

int main() {
    int n;

    printf("Enter a positive integer:");
    scanf("%d", &n);

    printf("Sum of first %d positive integers is %d\n", n, sum(n));
    printf("Sum of squares of first %d positive integers is %d\n", n, sum_squares(n));

    return 0;
}
```

Note that the above code uses two functions called `sum()` and `sum_squares()` that are not defined in the same file (we only declare their prototypes here). Hence, the `main.c` source file cannot be compiled directly to build an executable file. However, we can still build an object file from this source file. Roughly speaking, an object file contains relocatable addresses for functions. Hence, actual codes for the functions are not required to build object files. Different object files containing different functions can be linked together to create the final executable file. The object file for `main.c` can be built by typing the following command:
  
```
$ gcc -c main.c
```
 
This will create the object file called "`main.o`" in the same folder. Note that the `-c` option is used to tell the gcc compiler to stop after building the `.o` file so that it does not attempt to build an executable file at this stage. 

Next, create (in the same folder) two new source files containing the definitions of the functions `sum()` and `sum_squares()`. Call them `sum.c` and `sum_squares.c`, respectively. The files should contain the following code: 

**`sum.c`** 
 
```c
#include <stdio.h>
int sum(int m) {
    int i, sum = 0;

    for (i = 1; i <= m; i++) {
        sum += i;
    }

    return sum;
}
```
 
**`sum_squares.c`** 
 
```c
#include <stdio.h>

int sum_squares(int m) {
    int i, sum = 0;

    for (i = 1; i <= m; i++) {
        sum += i*i;
    }

    return sum;
}
```
 
Now, create the object files for each of these source files by typing: 
 
```
$ gcc -c sum.c sum_squares.c
```

We shall now link all the three object files that we have created so far to build our final executable file. This can be done using the following command: 
 
```
$ gcc main.o sum.o sum_squares.o -o numbers
```

This will create the a combined executable file called "`numbers`" by linking the three object files. Try running the executable file and see what it does. 

# Makefiles

The whole process of building an executable from its sources can be automated using _Makefiles_. A makefile contains recipes for building the final executable file as well as for each of the intermediate files. Each recipe is written in the following format: 
 
```makefile
target: dependencies
    action
```

where "`target`" is the name of the target you want to build; "`dependencies`" is the list of the files from which the "`target`" will be built; "`action`" specifies the command that must be executed to build the target from its dependencies. **Note:** the second line in each recipe must start with a _tab_ character followed by the action (do not use spaces instead of a tab!). In our case, the Makefile should look something as follows:

```makefile
numbers: main.o sum.o sum_squares.o
    gcc main.o sum.o sum_squares.o -o numbers

sum.o: sum.c
    gcc -c sum.c

sum_squares.o: sum_squares.c
    gcc -c sum_squares.c

main.o: main.c
    gcc -c main.c
```

Save the file as **Makefile**; again, don't forget the tabs in your makefile. You can run the makefile using the command:
 
```
$ make
```
 
from your terminal. This will build the entire project including all intermediate files.
 
> # Exercise 5
>
> For the same project, write another C source file that only contains a function `prime()`. This function should take an integer `n` as input and should output the sum of all primes from 2 to `n`. Call the `prime()` function from `main()` of `main.c` above to print the sum of the primes up to `n`.<br/><br/>
{: .block-danger } 

# Compile Time Debugging

Before we start looking at debugging a compiled application, let's have a look at some simple compile-time debugging. 

Should there be an error in your application source code, GCC will likely fail to compile and will provide information about the failure. On modern versions of GCC and Clang, these error messages explain the error and (usually) point to the line number and location of the error. For example, 
 
```
gcc -o test test.c
test.c:2:5: error: implicitly declaring library function 'printf' with type 'int (const char *, ...)' [-Werror,-Wimplicit-function-declaration]
    printf("%d", argc)
    ^
test.c:2:5: note: include the header <stdio.h> or explicitly provide a declaration for 'printf'
test.c:2:23: error: expected ';' after expression
    printf("%d", argc)
                      ^
                      ;
2 errors generated.
```
 
The output tells us that we have forgotten to include the `stdio` header file, but have used the `printf` function on line 2 (column 5), and that we missed a semi-colon on line 2 (column 23). 

However, it may be the case that there are minor issues in our code that do not prevent compilation, but may have an affect at runtime. Such "compiler warnings" may not be shown unless explicitly specified on the command line. 

Take the following code, for example: 
 
```c
#include <stdio.h>

int my_func() {
    printf("Hello World\n");
}

int main(int argc, char *argv[]) {
    my_func();
}
```
 
It might be hard to spot an issue with this code, and the compiler will likely compile the code. Some compilers (or versions of compiler) will output a warning, but some may not. 
 
```
$ gcc -o test test.c
$ ./test
Hello World

```
 
In the example, the `my_func()` function specified that it will return an `int`, but there is no `return` keyword in the function. While this doesn't harm the functionality in this case, it might cause undefined behaviour in a larger application. 

To find such issues at compile-time, we can either enable all warnings, or we can turn all warnings into error (such that an application doesn't compile if there are warnings present). 

We can enable displaying all warnings with the `-Wall` compile time flag. 
 
```
$ gcc -Wall -o test test.c
test.c: In function 'my_func':
test.c:5:1: warning: control reaches end of non-void function [-Wreturn-type]
    5 | }
      | ^
```
 
We can go further and prevent an application from compiling with warnings by using the `-Werror` compile time flag. 
 
```
$ gcc -Wall -Werror -o test test.c
test.c: In function 'my_func':
test.c:5:1: error: control reaches end of non-void function [-Werror=return-type]
    5 | }
      | ^
cc1: all warnings being treated as errors
```
 
Note that in the first case (with just `-Wall`), the code compiled successfully, and an executable binary is created. In the second case, the binary is not created and compilation has failed. 

# Debugging with GDB

Debugging C code is often one of the most frustrating periods of code development. While the Java runtime environment outputs nice and helpful errors, C programs often crash with a segmentation fault. These are usually caused by reading or writing unallocated or unowned memory. For instance, if we allocate space for 10 integers, and then try to write to the 10th index (i.e. the 11th element), the result is undefined. If you are lucky the application will crash and report a segmentation fault, but it may fail silently or cause random errors. 

To debug our applications we usually use the GNU Debugger (`gdb`). First we compile our application with the debug flag (`-g`) to include debugging information. 
 
```
$ gcc -g -o helloworld helloworld.c
```

Then we run the application with `gdb` like so: 
 
```
$ gdb ./helloworld
```

This presents us with a prompt in which to control our application. Type "`run`" and it will run your application. In the event of a fault, `gdb` will stop the program and return you to a prompt. Here you can interact with the operating system to find out what line of the application was being executed at the time and what the value of various variables were at the time. Consider the following example application: 
 
```c
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
   int *my_array = (int *) malloc(5 * sizeof(int)); 

   int i;

   for (i = 0; i < 1000000; i++) { 
      my_array[i] = i;
   }

   free(my_array);
}
```

If you write this application and compile it, upon execution you may see an error like the one shown below. Notice that the error was not triggered immediately -- if we had only gone a little past the end of our allocated memory the application would have continued, but we would still have overwritten some random piece of memory with garbage. This can have devastating consequences and lead to all sorts of nasty surprises. 
 
```
$ ./test.out
Segmentation fault: 5000
```
 
To debug this, we recompile the application but with the `-g` flag provided. We then run `gdb` on the application (as above). Type "`run`" into `gdb` and you may see an error like this:
 
```
Program received signal EXC_BAD_ACCESS, Could not access memory.
Reason: KERN_INVALID_ADDRESS at address: 0x0000000000000000
0x0000000100000f05 in main (argc=1, argv=0x7fff5fbffb90) at test.c:10
10                      my_array[i] = i;
```
 
This indicates that the error is caused at line 10 in `test.c`. Looking at line 10, we see that the error occurs when trying to store a value at a location in `my_array`. Since this memory access is invalid it seems likely that the reason the program crashing is because the memory has not been allocated. 

More information can be extracted using `gdb` by using the commands "`bt`" and "`print`". `bt` will produce a backtrace, which for our program was uninteresting, but would otherwise contain the call path to your function (e.g. if `main()` called `f1()`, which in turn called `f2()`, etc.). `print` can be used to print the values of in-scope variables at the time of error. Try typing `print i` into `gdb` and it will inform you that the program above crashed when the value of `i` is invalid. After you are done with debugging, to exit from the `gdb` prompt type "`quit`". 

`gdb` is a powerful debugger with a great number of features. This [cheat sheet](https://darkdust.net/files/GDB%20Cheat%20Sheet.pdf) demonstrates many of the most important features and may come in handy in the future. 

> **Note:** If you're using a Mac, it is likely that `gdb` will not be available. Instead you can use `lldb` 
{: .block-tip }

> # Exercise 6
>
> Fix the above program and ensure that it runs to completion (it should not produce any errors). <br/><br/>
{: .block-danger }


   
   
     
