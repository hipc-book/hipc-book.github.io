---
title: 2. Introduction to C (part 2)
date: 2022-07-21
category: hipc
layout: post
---


# Overview

In this practical we will cover: 
  
* Static and dynamic memory 
* Strings 
* File I/O 
* Structures 
* Debugging with valgrind or the address sanitizer
  

# More Memory Management

A compiled program's memory is divided into five segments: text, data, bss, heap, and stack. Each segment has a different purpose. The two segments which programmers are most concerned with are the stack and the heap. There are two ways memory can be acquired in C -- statically and dynamically -- which cause memory to be allocated on the stack and heap, respectively. 

In the previous practical you learnt about dynamic memory allocation, where memory is allocated in an on-demand fashion with the use of functions like `malloc()`. Dynamic memory is allocated on the heap and is limited only by the available memory on your computer. Blocks of memory in the heap segment can be allocated and used for whatever the programmer may need. Any memory allocated on the heap will stay around until it is released by calling `free()`. It is important that you free any memory you allocate with `malloc()`/`calloc()`/`realloc()` otherwise your application will have a "memory leak". We'll deal with how to debug memory leaks at the end of this practical. 
 
```c
int *dynamic_array = (int *) malloc(10 * sizeof(int)); // Dynamically allocated on heap, typecast to int *
...
free(dynamic_array); // release memory
```

Statically allocated memory lives on the program stack where space is limited by the operating system. The stack segment is used as a temporary scratch pad to store local function variables and context during function calls. Before calling a function a program must first build a stack frame which contains all of the function's arguments along with enough free space to hold any local variables declared by that function. This is pushed onto the stack and the processor jumps to the start of the function code. When a function returns, the stack frame is popped off the stack and discarded. This means that statically allocated memory only exists for the duration of a function call and is automatically allocated on function entry and released after the function exits. The compiler will automatically generate code to do this, meaning the programmer does not have to explicitly manage static memory. To statically allocate an array of integers in a function (as we've previously been doing), you can write:
 
 
```c
void foo(double argument) {
  int my_array[10]; // Static allocation; memory is made available on the stack. Fixed size.
  double a_number = argument;  // This is also static allocation.
}

int main(int argc, char *argv) {
  foo(42.0); // Stack frame is pushed on function entry and discarded after function exit
  // varibles a_number and my_array have ceased to exist here.
}
```

# Strings

Dealing with strings in Java is rather simple; however in C, strings don't _really_ exist. Instead, C deals with arrays of `char` elements. The string "hello, world" in C is simply an array of 13 characters ending in the _null_ terminator character (a zero byte, 0x00 in hex) which signifies that the string has ended: `["h", "e", "l", "l", "o", ",", " ", "w", "o", "r", "l", "d", 0x00]` 

Consider the following two examples, both concatenating two strings and then printing the result: 

**Java** 
 
```java
String my_string = "This is a string in ";
my_string = my_string + "Java";
System.out.println(my_string);
```
 
**C** 
 
```c
char *string_a = "This is a string in "; 
char *string_b = "the C language";
char *tmp_string = (char *) calloc(strlen(string_a) + strlen(string_b) + 1, sizeof(char)); // N.B. the +1 is for the null terminator.
strcat(tmp_string, string_a);
strcat(tmp_string, string_b);

printf("%s", tmp_string);
```
 
Clearly, concatenation is much more complicated in C, but we should try to understand it anyway! 

To get started with strings, we need to use the C string library ([`string.h`](http://linux.die.net/man/3/string)). Some of the most useful string functions are outlined in this practical, but you are advised to look at the string man page for a broader overview of its functions. 

In C, all strings must all end with a null character (`\0`) and you should therefore account for this when allocating memory. If a string pointer is assigned a value like in the above example (line 1), the pointer will point to its first character. The same is true for `tmp_string`, except this will reside on the heap (since the memory was reserved with `calloc()` first). 
 
```c
size_t strlen(const char *s);
```
 
The [`strlen()`](http://linux.die.net/man/3/strlen) function returns the number of characters in a string. This will likely be different to the amount of memory allocated for a particular string -- it will report the number of characters up to _(but not including)_ the null character.
 
In the above example, `strlen` is used to get the size of `string_a` and `string_b`. The space required for the concatenated string is then the sum of the length of these two strings plus 1 for the null character. 
 
```c
char * strcat(char * destination, const char * source);
```

The `strcat()` function appends a copy of the `source` string to the `destination` string. The terminating null character in `destination` is overwritten by the first character of `source`, and a null-character is included at the end of the new string formed by the concatenation of both in `destination`.
 
An alternative approach would be to use the `sprintf` function to amalgamate the two strings: 
 
```c
sprintf(tmp_string, "%s%s", string_a, string_b);
```

The `sprintf` function is much the same as the [`printf()`](http://linux.die.net/man/3/printf) function we encountered last time. Where `printf` prints to the standard output stream, [`sprintf()`](http://linux.die.net/man/3/sprintf) prints into memory (which must be allocated with at least enough room).
 
```c
int strcmp(const char *s1, const char *s2);
```
 
[`strcmp()`](http://linux.die.net/man/3/strcmp) compares two strings lexicographically (actually, it operates on the ASCII codes for each letter. This can cause some confusion, for example with capital Z considered to come before lowercase a) and returns a negative number if `s1` is less than `s2`, a positive number if `s1` is greater than `s2`, and 0 when the two strings are equal. For example: 
 
```c
char *s1 = "this is a string";
char *s2 = "this is a string";
char *s3 = "this is a third string";

int v = strcmp(s1, s2); // This will be 0 as they are equal

int w = strcmp(s1, s3); // This will be non-zero
```

The final function we will discuss in this practical is the [`strstr()`](http://linux.die.net/man/3/strstr) function.
 
```c
char *strstr(const char *haystack, const char *needle);
```
 
The `strstr()` function finds a substring within a larger string and returns a pointer to the beginning of the substring, or NULL if the substring is not found. For example:
 
 
```c
char *s1 = "this is a string";
char *s2 = "is a";
char *s3 = "balloon";

char *r1 = strstr(s1, s2); // This will return a pointer to: "is a string"

char *r2 = strstr(s1, s3); // This will return NULL
```
 
Like in the previous practical, memory leaks and buffer overflows are serious problems when dealing with strings stored on the heap. You **must** make sure that when you allocate memory for a string, you **include 1 additional character space for the NULL character** and that you don't overrun the memory you have allocated.

A buffer overflow occurs when you write more data to memory than you have allocated space for. While your application will often not crash when doing this operation, it may cause issues at a later time as you may overwrite memory being used for something else entirely. Furthermore, buffer overflows are often the cause of security problems in applications. If you can overwrite memory in the correct way, it is actually possible to inject malicious assembly code into a running application. 

> # Exercise 1
>
> Complete the following program by writing the code for the function `count_words()` that takes a string as input and returns the number of words in the string 
>  
> ```c
> #include <stdio.h>
> #include <string.h>
> 
> int count_words(char *str) {
>     // this function should return the number of words in str
> }
> 
> int main() {
>     char str[100];
> 
>     printf("Enter a string:");
>     fgets(str, 100, stdin);    // this function reads a line or at most 99 bytes from stdin file stream that represents the keyboard
> 
>     printf("Number of words in the entered string is %d\n", count_words(str));
> 
>     return 0;
> 
> }
> ```
>  
> Note that the `main()` function in the above exercise uses the function `fgets()` to read characters from the [`stdin`](https://linux.die.net/man/3/stdin) file stream. In C, a file stream is used to represent an opened I/O device or file. The `stdin` file stream represents the keyboard. Also, note that `fgets()` in the above program will read at most 99 bytes (one less than the second argument) from the `stdin` file stream leaving 1 byte to store the null character (`'\0'`) which makes '`str`' a valid string. See the man page of [`fgets()`](https://linux.die.net/man/3/fgets) to know more about the function.<br/><br/>
{: .block-danger }
 
# File I/O

There are two mechanisms to handle files in C. These are: 

1. Using **File Streams**
2. Using **File Descriptors**

## File Streams

A file stream is a pointer of type [`FILE`](https://linux.die.net/man/3/stdio) which is a predefined data type in the C standard I/O library. It is used to represent an opened file. Consider the following program in which a text file is opened, read, and its contents are displayed to the terminal. 
 
```c
#include <stdio.h>
#include <stdlib.h> // required for the exit() function

#define BUFSIZE 10

int main() {
    FILE *fptr1 = fopen("./textfile1.txt", "r");

    if(fptr1 == NULL){
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }

    char str[BUFSIZE];
    printf("Content of the opened file:\n");

    while(fgets(str, BUFSIZE, fptr1) != NULL) { // read the file line by line
        fprintf(stdout, "%s", str);             // same as doing printf("%s",str);
    }

    printf("\n");
    fclose(fptr1);

    return 0;
}
```

Here `fopen()` is used to open the file `"textfile1.txt"` in read mode (specified by the second argument `"r"`). For more details on the different modes available to open a file see the man page of [`fopen()`](http://linux.die.net/man/3/fopen). If successful, `fopen()` returns a `FILE` pointer that represents the opened file stream. In this case, the pointer `fptr1` represents the file stream and all subsequent handling of the file is done using this pointer. If `fopen()` is not successful in opening the file stream (for example, if the file does not exist), then it returns `NULL` (in which case we print an error message and exit). 

In the example, we use the `fgets()` function repeatedly (until we reach `EOF`, or the End-of-File character, in which case `fgets()` returns `NULL`) to read from the file stream `fptr1`. Each time it reads a line, or `BUFSIZE-1` bytes, from the file stream and stores them in `str`. We then display the string `str` to the terminal (which is represented by the file stream [`stdout`](https://linux.die.net/man/3/stdout)) using the [`fprintf()`](https://linux.die.net/man/3/fprintf) function. Each time `fgets()` is executed, the reading position indicator of the file stream is advanced by the number of bytes read and points to the next input character in the file stream. When reading is finished, we close the file using [`fclose()`](https://linux.die.net/man/3/fclose). 

One important point to remember is that each C program has, by default, three standard I/O streams opened (no need to use `fopen()` to open these file streams). These standard I/O streams are `stdin`, `stdout`, and `stderr`, which are used to read from the keyboard and to display output and errors to the terminal. Hence, the line `fprintf(stdout, "%s", str)` will do the same job as `printf("%s", str)`. 

## File Descriptors

So far we have seen how to handle files using file streams. A file stream is a high-level data structure defined in the C standard I/O library. For UNIX-based operating systems, there is another more primitive way of handling files using file descriptors. File descriptors are nothing but integers to represent files. The standard I/O streams `stdin`, `stdout`, and `stderr` have file descriptors 0, 1, and 2, respectively. Any additional files can be opened using the [`open()`](http://linux.die.net/man/3/open) function which returns a file descriptor for the corresponding file. We can read from or write to a file descriptor using the [`read()`](http://linux.die.net/man/3/read) and [`write()`](http://linux.die.net/man/3/write) system calls. Consider, for example, the following program which does the same job as the previous program but using file descriptors instead of file streams: 
 
```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>  // for the open() function
#include <unistd.h> // for read(), write() and close() functions

#define BUFSIZE 10

int main() {
    int fd1 = open("./textfile1.txt", O_RDONLY);  // opens the file in read only mode and returns a file descriptor for the file
    int x;

    if (fd1 < 0) {
        write(2, "Error opening file\n", sizeof("Error opening file\n")); // writes the error message to stderr
        exit(1);
    }

    char str[BUFSIZE];

    printf("Content of the opened file:\n");

    while((x = read(fd1, str, BUFSIZE-1)) > 0){ // read() reads up to BUFSIZE-1 bytes from fd1 and stores in str
                                                // the number of bytes actually read is returned
            str[x] = '\0';
            write(1, str, strlen(str));   // '1' is the file descriptor for stdout, hence this is equivalent to printf("%s", str)
    }

    printf("\n");
    close(fd1);

    return 0;

}
```
 
At this point, you may be asking what is the advantage of knowing both mechanisms of file handling described above when the job can be done using either of them. In most cases, normal file handling and I/O operations are handled very efficiently by file stream operations (such as `fopen()`, `fgets()`, `fread()`, `fwrite()`, etc.) defined in the C standard I/O library. These operations actually use the more basic system calls `open()`, `read()`, and `write()` to do their job.  

However, the standard I/O library functions use buffers to reduce the number of system calls so that context switching time is minimised. As a result, in some cases data will not be immediately read or written when using file stream operations as these operations may store the data in a buffer to be read or written later. However, some applications may require data to be read or written immediately and not buffered. In such cases, to have more control on the flow of data, we could use file descriptors along with `read()` and `write()` system calls. 

> # Exercise 2
>
> Write a C program that copies the contents of one text file into another. You should write two versions of the program; one using file streams only and another using file descriptors only. <br/><br/>
{: .block-danger }

# Structures

Structures, or '`structs`', are a mechanism for defining complex data types in C. They allow a programmer to combine a group of related variables to be placed in a contiguous block of memory. The `struct` shares a superficial similarity with Java's objects in that they allow a programmer to model a complex structure as a single entity. That said, `structs` are not objects as they cannot contain functions, have no mechanism for inheritance or polymorphism, and do not support encapsulation. 

**Java** 
 
```java
public class Book {
  private String title;
  private double price;

  public Book(String title, double price) {
    this.title = title;
    this.price = price;
  }

  public double getPrice() { 
    return price; 
  } 

  public void setPrice(double price) { 
    this.price = price;
  }

  ...
}

...

Book b = new Book ("Charlotte's Web", 10.88);
b.setPrice(10.99);
```
 
**C** 
 
```c
struct Book {
  char *title;
  double price;
}; // Remember the semicolon!

...

struct Book b = {"Charlotte's Web", 10.88};
b.price = 10.99;
```
 
Struct members are accessed with the dot notation: `struct.member`. Structs are used to implement complex data structures in C. The following listing demonstrates a partial linked list implementation: 
 
```c
#include <stdio.h>
#include <stdlib.h>

struct element {
  struct element * next;
  int data;
};

struct linked_list {
  struct element * head;
};

void append_int(struct linked_list * list, int val) {
  struct element * elem = malloc(sizeof(struct element));

  elem->data = val;
  elem->next = NULL; // Really important to explicitly set this to null. Malloc does not zero memory

  if (list->head == NULL) {
    // Empty list, we need to append to head
    list->head = elem;
  } else {
    // List has some elements, find the end and append to that
    struct element * tail = list->head;
    while (tail->next != NULL) {
      tail = tail->next;
    }
    tail->next = elem;
  }
}
```

This code should remind you of a standard linked list implementation. Note however that as C lacks objects, the methods to update the list are external and not part of the element or `linked_list` structs. Another key point to notice in this listing is the introduction of the crows-foot operator, `->`. This operator is syntactic sugar to allow structure members to be accessed through pointers. To understand this, look at the following listing. Both lines are equivalent and have the same result, but the crows-foot allows for cleaner code. 
 
```c
tail = tail->next;   // the crow's foot is a pretty shortcut for..

tail = (*tail).next; // this ugly code.
```
 
The second line dereferences the tail pointer (in brackets), and having dereferenced it accesses the next member with the standard dot syntax.

> # Exercise 3
>
> Extend the above example to allow elements to be inserted and removed at the head of the list.<br/><br/>
{: .block-danger }

> # Exercise 4
>
> Using structs, implement a queue data structure with functions to check if the queue is empty (`isempty`), to insert a node to the tail end of the queue (`enqueue`), and to remove a node from the head of the queue (`dequeue`).<br/><br/>
{: .block-danger }

# More Debugging

Debugging your application when a segfault occurs can usually be done quite successfully using `gdb` (though not always!). But consider the following application: 
 
```c
#include <stdlib.h>

int main(int argc, char *argv[]) {
   int *array_of_ints = (int *) malloc(sizeof(int) * 10);
   int i;

   for (i = 0; i <= 10; i++) {
      array_of_ints[i] = i;
   }
}
```
 
You might already be able spot the error in the code above. If you were to compile and run the above application, it would run to completion and nothing seems amiss; however if this was part of a larger application, this code may well cause a segfault somewhere else in the code entirely. To debug these problems, there are two options. 

One is a tool called `valgrind`. If we compile the above program using debug symbols (i.e. with `-g`) and the run it through `valgrind`:
 
```
$ valgrind ./test
```
 
Valgrind will produce output like the following: 
 
```
==16358== Memcheck, a memory error detector
==16358== Copyright (C) 2002-2010, and GNU GPL'd, by Julian Seward et al.
==16358== Using Valgrind-3.6.0.SVN-Debian and LibVEX; rerun with -h for copyright info
==16358== Command: ./test
==16358==
==16358== Invalid write of size 4
==16358== at 0x40051A: main (test.c:7)
==16358== Address 0x518b068 is 0 bytes after a block of size 40 alloc'd
==16358== at 0x4C244E8: malloc (vg_replace_malloc.c:236)
==16358== by 0x4004FC: main (test.c:4)
==16358==
==16358==
==16358== HEAP SUMMARY:
==16358== in use at exit: 40 bytes in 1 blocks
==16358== total heap usage: 1 allocs, 0 frees, 40 bytes allocated
==16358==
==16358== LEAK SUMMARY:
==16358== definitely lost: 40 bytes in 1 blocks
==16358== indirectly lost: 0 bytes in 0 blocks
==16358== possibly lost: 0 bytes in 0 blocks
==16358== still reachable: 0 bytes in 0 blocks
==16358== suppressed: 0 bytes in 0 blocks
==16358== Rerun with --leak-check=full to see details of leaked memory
==16358==
==16358== For counts of detected and suppressed errors, rerun with: -v
==16358== ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 4 from 4)
```

This actually informs us there are 2 problems in the above code snippet. Firstly, the leak summary informs us that we lose 40 bytes of memory in our application. Lost memory occurs when memory is allocated but not freed, therefore to fix this problem we must free `array_of_ints` at the end of the application. Secondly, there is an invalid write (of size 4). This occurs because we are writing too many pieces of data into the `array_of_ints`. 

The second option is to enable the address sanitizer at compile time with `-fsanitize=address`. 
 
```
$ gcc -fsanitize=address -g -Wall -o test test.c
$ ./test
=================================================================
==25395==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x604000000338 at pc 0x00010b258f02 bp 0x7ff7b4caa5c0 sp 0x7ff7b4caa5b8
WRITE of size 4 at 0x604000000338 thread T0
    #0 0x10b258f01 in main test.c:8
    #1 0x10ba204fd in start+0x1cd (dyld:x86_64+0x54fd)

0x604000000338 is located 0 bytes to the right of 40-byte region [0x604000000310,0x604000000338)
allocated by thread T0 here:
    #0 0x10bb17597 in wrap_malloc+0xb7 (libasan.6.dylib:x86_64+0x4c597)
    #1 0x10b258ea2 in main test.c:4
    #2 0x10ba204fd in start+0x1cd (dyld:x86_64+0x54fd)

SUMMARY: AddressSanitizer: heap-buffer-overflow test.c:8 in main
Shadow bytes around the buggy address:
  0x1c0800000010: fa fa 00 00 00 00 00 00 fa fa 00 00 00 00 00 00
  0x1c0800000020: fa fa 00 00 00 00 00 05 fa fa 00 00 00 00 00 00
  0x1c0800000030: fa fa 00 00 00 00 00 05 fa fa 00 00 00 00 00 00
  0x1c0800000040: fa fa 00 00 00 00 00 07 fa fa 00 00 00 00 00 00
  0x1c0800000050: fa fa 00 00 00 00 00 00 fa fa 00 00 00 00 00 05
=>0x1c0800000060: fa fa 00 00 00 00 00[fa]fa fa fa fa fa fa fa fa
  0x1c0800000070: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c0800000080: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c0800000090: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c08000000a0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c08000000b0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
  Shadow gap:              cc
==25395==ABORTING
[1]    25395 abort      ./test
```
 
Again the output informs us that we have overrun our allocated memory on line 8. 

> **Further Reading**
> 
> * [GCC Instrumentation Options](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html) 
{: .block-tip }

> # Exercise 5
>
> Fix the errors in the above application and then recompile and rerun through `valgrind` (or with the address sanitizer). What is the result? <br/><br/>
{: .block-danger }
   
     