# Left-Right Concurrency Control

This is an implementation of left-right concurrency control in C++. The
concurrency control mechanism is by Pedro Ramalhete and Andreia Correia. My
contribution is solely an implementation that makes use of modern C++ features.

As I write this, papers on this concurrency control technique are forthcoming.
For the time being the best reference is probably
http://concurrencyfreaks.blogspot.com/2013/12/left-right-concurrency-control.html

This is a header-only library. Put the leftright.hpp file somewhere that your
C++ compiler can find it and you're good to go.

## Summary of features

The basic idea is to provide a class template that can wrap _any_ other
class so as to provide wait-free population-oblivious concurrent read accesses
without requiring the complexities of SMR. The cost of the left-right mechanism
is two-fold: (1) the memory requirements for your datastructure double because
two copies of it are kept, and (2) updates are threadsafe but are blocking (but
still do not interfere with reads). Consequently this class and left-right in
general is best used for small data structures where reads dominate writes.

You can see usage examples in the unit tests. There are really only two member
functions on the leftright class template: observe and modify. The observe
function takes a unary functor and executes it, providing it a const reference
to the underlying datastructure. The modify function takes a noexcept unary
function and executes (twice) against a non-const reference to the underlying
datastructure. It is vitally important that the functors provided to the modify
function be deterministic and make exactly the same modifications to the
datastructure for each invocation.

## Building

There's really no build needed as this is a header-only library, however if you
want to run the unit tests or generate docs you can use the cmake build. To
perform an out-of-tree build

    $ cd /tmp # or wherever
    $ mkdir leftright-debug
    $ cd leftright-debug
    $ cmake -DCMAKE_BUILD_TYPE=debug /path/to/leftright/repository
    $ make && make test
    $ make docs # generates doxygen documentation under /tmp/leftright-debug/docs
    
