1. Overview
===========

This coursework explores parallelism using threaded-building
blocks. You should (if you're doing it right), see at least some
examples of _linear speedup_ here, i.e. the performance
rises in proportion to the number of CPU cores.

This distribution contains a basic object framework for creating
and using fourier transforms, as well as two implementations:

1. A direct fourier transform taking O(n^2) steps.
2. A recursive fast-fourier transform taking O(n log n) steps.

Also included within the package is a very simple test suite
to check that the transforms work, and a registry which allows
new transforms to be added to the package.

Your job in this coursework is to explore a number of basic
(but effective) ways of using TBB to accelerate existing code,
and to compare and contrast the performance benefits of them.

_Dislaimer: Never write your own fourier transform for production
use. Everything you do here will be slower and less accurate than
existing FFT libraries._

1. Evironment and setup
=======================

Setting up TBB
--------------

You can download Threaded Building Blocks from the [TBB Website](https://www.threadingbuildingblocks.org/)
or it is available for many package managers

At the time of writing the stable version is 4.2, which is what
I will be using in the tests for the coursework. That said, I have no
expectations of incompatibility with the later version which I now
see is available. You have the choice of downloading and building them
from source (you have knowledge of how to do that via configure/make), or you
can install the binary packages on your machine.

The target environment for this coursework is
Ubuntu Server 14.04, specifically the Amazon AWS AMI
version of Ubuntu 14.04. What you choose to do the
development in is up to you, but the assessed compilation
and evaluation will be done under Ubuntu in AWS. My plan
is to use a [c4.8xlarge](http://aws.amazon.com/ec2/instance-types/)
instance, but you certainly shouldn't optimise your code
specifically for that machine. This shouldn't matter
to you now (as you're not relying on anything apart from
TBB), but is more encouragement to try out AWS before
you have to in the next coursework.

Submission will be via github, so one thing you need
to do is become part of the HPCE github organisation. This
will give you access to a private repository that you can
push to, and I can pull from. During submission I will be
doing a git clone of your repository, then building it in
place. To get access (do this now):

- Get a github account (if you don't have one).
- Note down your github id and your imperial login (e.g. mine is dt10).
- Send an email to me, from an Imperial email account,
  with the title "[HPCE-github-request]". The body should
  consist of your github id and your imperial login, so
  something like:
  
      github: m8pple
      imperial: dt10

I will then add you to the HPCE organisation, and you will
get a private repository. I'll be doing this manually and in
batches, so it may take a day or so to turn it around. So
do it when you read this.

_Note: Again, it is not a disaster if submission via github
doesn't work, I will also be getting people to do a blackboard
submission as backup so I get the code. (Also as a hedge in case
it doesn't work my end, as I haven't tried it before)._

Included in this package is a `makefile`, which may get you
started in posix, and `makefile.mk`, which will allow you
to get started in windows with `nmake` (the `make` that comes
with visual studio). Note that `nmake` is much less powerful
than GNU `make`, and cannot do many of the more advanced
things such as parallel build, or using eval to create
rules at make-time.

When I build your code, I will not be using your makefiles,
and will be compiling your .cpp files directly. There will
be a copy of TBB 4.2 available in the environment that your
code will use, but exactly how that happens is opaque to
you.

The fourier transform framework
-------------------------------

The package you have been given is a slightly overblown
framework for implementing fourier transforms. The file
`include\fourier_transform.hpp` defines a generic
fourier transform interface, allowing forwards and
backwards transforms.

In the src directory, there are two fourier transform
implementations, and two programs:

1. test_fourier_transform, which will run a number of
   simple tests on a given transform to check it works.
   The level of acceptable error in the results is
   defined as 1e-9 (which is quite high).
   
2. time_fourier_transform, which will time a given
   transform for increasing transform sizes with a given
   level of allowed parallelism.

If you build the two programs (using the appropriate
makefile, or some other mechanism), then you should
be able to test and time the two existing transforms.
For example, you can do:

    test_fourier_transform

which will list the two existing transforms, and:

    time_fourier_transform hpce.fast_fourier_transform

to see the effect of increasing transform size on
execution time (look in `time_fourier_transform.cpp` to
see what the columns mean).

Even though there is no parallelism in the default
framework, it still relies on TBB to control parallelism,
so it will not build, link, or execute, without TBB
being available.

_Note: the direct fourier transform is very likely
to fail some tests, while the fast fourier transform
(should) pass all of them. This demonstrates that
even though two algorithms calculate the same result,
the order of calculation can change the numerical
accuracy significantly. In case you are not aware,
floating point is [not exact](http://docs.oracle.com/cd/E19957-01/806-3568/ncg_goldberg.html),
and if you add together long sequences the error
adds up quickly._

You may wish to explore the algorithmic tradeoffs
between the sequential direct and fast fourier
transforms - remember that parallelism and optimisation
are never a substitute for an algorithm with a better
intrinsic complexity.

2. Using tbb::parallel_for in direct_fourier_transform
======================================================

The file `src/direct_fourier_transform.cpp` contains a classic
discrete fourier transform, which takes O(n^2) operations to
do an n-point fourier transform. The structure of this
code is essentially:

    for a=0:n-1
      out[a]=0;
      for b=0:n-1
        out[a]=out[a] + in[i] * exp(-i * 2*pi * a * b / n);
      end
    end

This should remind you of roots of unity and such-like
from way back when.

Overview of tbb::parallel_for
-----------------------------

There are two loops here, with a loop-carried dependency
through the inner loop, so a natural approach is to
try to parallelise over the outer loop. In matlab this
would be inefficient using parfor, except for very large
transforms, but in TBB the overhead is fairly small.

The parfor equivalent is given by tbb::parallel_for,
which takes a functor (function-like object) and applies
it over a loop. The simplest approach is to take a
loop which looks like:

    for(unsigned i=0;i<n;i++){
      f(i);
    }

and turn it into:

    tbb::parallel_for(0u, n, f);

This approach works, but with C++11 lambda functions
we can make it look nicer. So we can take:

    for(unsigned i=0;i<n;i++){
      y[i]=f(x[i]);
    }

and turn it into:

    #include "tbb/parallel_for.h"

    tbb::parallel_for(0u, n, [=](unsigned i){
      y[i]=f(x[i]);
    });

If you have problems compiling this, and get loads of cryptic error
messages, it may be because the types passed to the function
don't agree. In particular, both the begin and end of the iteration
range should have the same type. In the example above, `0u` has
the type `unsigned`, so if `n` had the type `int`, there might
be a type error (as the iteration type TI can't be both `int` and
`unsigned`). To resolve it you can either make sure both parts
have the same time:

    tbb::parallel_for((unsigned)0, (unsigned)n, [=](unsigned i){
          y[i]=f(x[i]);
    });

or you can explicitly state the iteration type:

    tbb::parallel_for<unsigned>(0, n, [=](unsigned i){
          y[i]=f(x[i]);
    });


Despite the way it initially appears, `parallel_for` is
still a normal function, but we just happen to be passing
in a variable which is code. The [=] syntax is introducing
a lambda, in a very similar way to the @ syntax in matlab,
so we could also do it in a much more matlab way:

    auto loop_body = [=](unsigned i){
      y[i]=f(x[i]);
    };
    tbb::parallel_for(0u, n, loop_body);

This is not the only form of `parallel_for`, but it is
the easiest and most direct. Other forms allows for
more efficient execution, but require more thought.

Using tbb::parallel_for in the fourier transform
------------------------------------------------

The framework is designed to support multiple fourier
transforms which you can select between, so we'll
need a way of distuinguishing your transform from
anyone elses (in principle I should be able to create
one giant executable containing everyone in the
class's transforms). The basic framework uses the namespace
`hpce`, but your classes will live in the namespace
`hpce::your_login`, and the source files in `src/your_login`.
For example, my namespace is `hpce::dt10`, and my
source files go in `src/dt10`.

There are five steps in this process:

1. Creating the new fourier transform class
2. Registering the class with the fourier framework
3. Adding the parallel constructs to the new class
4. Testing the parallel functionality (up to you)
5. Finding the new execution time (up to you, see notes at the end).

### Creating the new fourier transform class

Copy `src/direct_fourier_transform.cpp` into a new
file called `src/your_login/direct_fourier_transform_parfor.cpp`.
Modify the new file so that the contained class is called
`hpce::your_login::direct_fourier_transform_parfor`, and reports
`hpce.your_login.direct_fourier_transform_parfor` from `name()`. Apart
from renaming, you don't need to change any functionality yet.

To declare something in a nested namespace, simply
insert another namespace declaration inside the existing
one. For example, if you currently have `hpce::my_class`:

    namespace hpce{
	  class my_class{
	    ...
	  };
	};
	
you could get it into a new namespace called bobble, by
changing it to:

    namespace hpce{
	  namespace bobble{
	    class my_class{
		  ...
		};
	  };
	};
	
which would result in a class with the name `hpce::bobble::my_class`.

Add your new file to the set of objects (either by adding
it to FOURIER_IMPLEMENTATION_OBJS in the makefile, or by
adding it to your visual studio project), and check
that it still compiles.

### Register the class with the fourier framework

As part of the modifications, you should have found
a function at the bottom of the file called `std::shared_ptr<fourier_transform> Create_direct_fourier_transform()`,
which you modified to `std::shared_ptr<fourier_transform> Create_direct_fourier_transform_parfor()`.
This is the factory function, used by the rest of the
framework to create transforms by name, without knowing
how they are implemented.

If you look in `src/fourier_transform_register_factories.cpp`, you'll
see a function called `fourier_transform::RegisterDefaultFactories`,
which is where you can register new transforms. To minimise
compile-time dependencies, the core framework knows nothing
about the transforms - all it knows is how to create them.

Towards the top is a space to declare your external factory
function, which can be uncommented. Then at the bottom
of `RegisterDefaultFactories`, uncomment the call which
registers the factory.

At this point, you should find that your new implementation
is listed if you build `test_fourier_transform` and do:

    test_fourier_transform hpce.your_login.direct_fourier_transform_parfor

Hopefully your implementation still works, in so far as the
execution will be identical.

If your transform doesn't turn up at run-time, or the code won't compile,
make very sure that you have renamed everything within
`src/your_login/direct_fourier_transform_parfor.cpp` to the
new name. Also make sure that the factory function is declared
as `std::shared_ptr<fourier_transform> hpce::your_login::Create_direct_fourier_transform_parfor()`,
both in `src/your_login/direct_fourier_transform_parfor.cpp`, and in 
`src/fourier_transform_register_factories.cpp` (particularly if you get a linker error).

### Add the parallel_for loop

You need to rewrite the outer loop in both `forwards_impl` and `backwards_impl`,
using the transformation of for loop to `tbb::parallel_for` shown previously. I would
suggest doing one, running the tests, and then doing the other. You'll
need to make sure that you include the appropriate header for parallel_loop from
TBB at the top of the file, so that the function can be found. The linker path
will also need to be setup using whatever build tool you are using (e.g. the
settings in visual studio, or LDFLAGS in a makefile).


3. Using tbb::task_group in fast_fourier_transform
==================================================

The file `src/fast_fourier_transform.cpp` contains a radix-2
fast fourier transform, which takes O(n log n) operations to
do a transform. There are two main opportunities for parallelism
in this algorithm, both in `forwards_impl()`:

1. The recursive splitting, where the function makes two recursive
   calls to itself.
2. The iterative joining, where the results from the two recursive
   calls are joined together.

We will first exploit just the recursive splitting using
`tbb::task_group`.

Overview of tbb::task_group
---------------------------

Task groups allow us to specify sets of heterogenous
tasks that can run in parallel - by heterogenous, we
mean that each of the tasks can run different code and
perform a different function, unlike `parallel_for` where
one function is used for the whole iteration space.

Task groups are declared as an object on the stack:

    #include "tbb/task_group.h"

    tbb::task_group group;

you can then add tasks to the group dynamically,
using anything which looks like a nullary (zero-input)
function as the starting point:

    unsigned x=1, y=2;
    group.run( [&x](){ x=x*2; } );
	group.run( [&y](){ y=y+2; } );

After this code runs, we can't say anything about the
values of x and y, as each one has been captured by
reference (the [&]) but we don't know if they have been
modified yet. It is is possible that zero, one, or
both of the tasks have completed, so to rejoin the
tasks and synchronise we need to do:

    group.wait();

After this function executes, all tasks in the group
must have finished, so we know that x==2 and y==4.

An illegal use of this construct would be for both
tasks to have a reference to x:

    // don't do this
    unsigned x=7;
    group.run( [&x](){ x=x*2; } );
	group.run( [&x](){ x=x+2; } );
    group.wait();

Because both tasks have a reference to x, any changes
in one task are visible in the other task. We don't know
what order the two tasks will run in, so the output
could be one of:

- x == (7*2)+2
- x == (7+2)*2
- x == 7*2
- x == 7+2

This would be a case of a data-race condition, which is
why you should never have two threads sharing the same
memory.

### Create and register a new class 

Copy `src/fast_fourier_transform.cpp` into a new
file called `src/your_login/fast_fourier_transform_taskgroup.cpp`.
Modify the new file so that the contained class is called
`hpce::your_login::fast_fourier_transform_taskgroup`, and reports
`hpce.your_login.fast_fourier_transform_taskgroup` from name(). Apart
from renaming, you don't need to change any functionality yet.

As before, register the implementation with the implementation
in `src/fourier_transform_register_factories.cpp`, and check that
the transform still passes the test cases.

### Use tbb::task_group to parallelise the recursion

In the fast fourier transform there is a natural splitting
recursion in the section:

    forwards_impl(m,wn*wn,pIn,2*sIn,pOut,sOut);
    forwards_impl(m,wn*wn,pIn+sIn,2*sIn,pOut+sOut*m,sOut);

Modify the code to use tbb::task_group to turn the two
calls into child tasks. Don't worry about efficiency
yet, and keep splitting the tasks down to the point of
individual elements - splitting down to individual
elements is the wrong thing to do, as it introduces
masses of overhead, but we are establishing a base-case here.

As before, test the implementation to make sure it still
works.

4. Exploring the grain size of parallel_for
===========================================

We are now going to explore tuning the grain size,
which is essentially adjusting the computation to
parallel overhead ration.

Partitioners and grain size
---------------------------

By default, `tbb::parallel_for` uses something called
the `auto_partitioner`, which is used to [partition](https://www.threadingbuildingblocks.org/docs/help/reference/algorithms/partitioners.htm)
your iteration space into sequential segments. The auto-partitioner
attempts to balance the number of parallel tasks created
against the amount of computation at each iteration
point. Internally it tries to split the iteration space
up recursively into parallel chunks, and then switches
to serial execution within each chunk once it has enough
parallel tasks for the number of processors.

In our direct FFT the amount of work within each parallel
iteration will depend on the transform size, as the total
cost is O(n^2), and the cost per inner loop iteration is
O(n). As the transform gets larger, the total amount of
true work is scaling as O(n^2), while the potential overhead
due to parallelism is scaling at O(n). Putting numbers on
this, if we assume t_c is the cost per data-point, and
t_p is the extra overhead per parallel task, then the overall
cost is t_p n + t_c n^2. Asmptotically this becomes O(n^2),
but for finite sizes we know that t_p >> t_c (by a factor
of around 1000), so for n~1000 the overhead will equal the
actual compute time.

We can explore this and control this by using manual
grain size control, which explicitly says how big each
parallel task should be. There is an alternate form
of parallel_for, which describes a chunked iteration
space:

    tbb::parallel_for(tbb::blocked_range<unsigned>(i,j,K), const blocked_range<int> &chunk){
        for(unsigned i=chunk.begin(); i!=chunk.end(); i++ ){
            y[i]=myLoop(i);
        }
    }, tbb::simple_partitioner);

this is still equivalent both to the original loop, but
now we have more control. If we unpack it a bit, we
could say:

    // Our iterations space is over the unsigneds
    typedef tbb::blocked_range<unsigned> my_range_t;
    
    // I want to iterate over the half-open space [i,j),
    // and the parallel chunk size should be K.
    my_range_t range(i,j,K);
    
    // This will apply my function over the half-open
    // range [chunk.begin(),chunk.end) sequentially.
    auto f=[&](const my_range_t &chunk){
        for(unsigned i=chunk.begin(); i!=chunk.end(); i++ ){
            y[i]=myLoop(i);
        }
    };
    
We now have the choice of executing it directly:

    f(range); // Apply f over range [i,j) sequentially

or in parallel with chunk size of K:

    tbb::parallel_for(range, f, tbb::simple_partitioner());

The final `tbb::simple_partitioner` argument is telling
TBB "I know what I am doing; I have decided that K is the
best chunk size." We could alternatively leave it blank,
which would default to the auto partitioner, which would
tell TBB "I know that the chunk size should not be less than
K, but if you want to do larger chunks, go for it.".

Environment Variables
---------------------

We want to set a good chunk size, but we also want it to be
user tunable for a specific machine. So we are going to allow the
user to choose a value K at run-time using an environment variable.

The function [`getopt`](http://www.cplusplus.com/reference/cstdlib/getenv/)
allows a program to read [environment variables](http://en.wikipedia.org/wiki/Environment_variable)
at run-time. So if I choose an environment variable called HPCE_X, I could
create a C++ program `read_x`:

    #include <cstdlib>
    
    int main()
    {
        char *v=getenv("HPCE_X");
        if(v==NULL){
            printf("HPCE_X is not set.\n");
        }else{
            printf("HPCE_X = %s\n", v);
        }
        return 0;
    }
    
then on the command line I could do:

    > ./read_x
    
      HPCE_X is not set.
   
    > export HPCE_X=wibble
    > ./read_x
    
      HPCE_X = wibble
    
    > export HPCE_X=100
    > ./read_x
    
      HCPE_X = 100

Environment variables are a way of defining ambient properties,
and are often used for tuning the execution of program for the
specific machine they are on (I'm not saying it's the best way,
but it happens a lot). I used the "HPCE_" prefix as I want to
try to avoid clashes with other programs that might want a
variable called "X".

### Create and register a new class

Create a new class `src/your_login/direct_fourier_transform_chunked.cpp` based
on `src/your_login/direct_fourier_transform_parfor.cpp`
with class name `hpce::your_login::direct_fourier_transform_chunked`, and name
`hpce.your_login.direct_fourier_transform_parfor_chunked`.

### Add parameterisable chunking to your class

Change the parfor loop to use a chunk size K, where K is either
specified as a decimal integer using the environment
variable `HPCE_DIRECT_OUTER_K`, or if it is not set, you should use
a sensible default. For example, if the user does:

    export HPCE_DIRECT_OUTER_K=16

you would use a chunk size of 16 (which will work well for larger
n, but less well as n becomes smaller).

You should think about what "sensible default" means: the TBB user
guide gives [some guidance](https://www.threadingbuildingblocks.org/docs/help/tbb_userguide/Controlling_Chunking.htm)
in the form of a "rule of thumb". Your rule of thumb
should probably take into account the size of the inner loop:
remember that the work is O(n^2), and the work within each
original parfor iteration is O(n). But on the other side,
you always want enough tasks to keep the processors occupied
(no matter how many there are), so you can't set the chunk size to K=n.

5. Adjustable grain size for the FFT
====================================

Our recursive parallel FFT is currently splitting down to
individual tasks, which goes completely against the
TBB advice about the minimimum work per thread.

Go back into `src/your_login/fast_fourier_transform_taskgroup.cpp`
and make the base-case adjustable using the environment variable
`HPCE_FFT_RECURSION_K`. This should adjust things at run-time,
so that if I choose:

    export HPCE_FFT_RECURSION_K=1

then the work is split down to individual tasks, so the run-time
should be very similar to the original, while if we do:

    export HPCE_FFT_RECURSION_K=16
    
then the implementation will stop parallel recursion for
at a size of 16, and switch to serial recursion (i.e. normal
function calls).

You don't want the overhead of calling `getenv` all over the
place, so I would suggest calling it once at construction
time and caching it in a member variable for the lifetime
of the FFT instance.

_Note: this is an in-place modification, rather than a new class._

6. Using parallel iterations in the FFT
=======================================

Making the loop parallelisable
------------------------------

The FFT contains a for loop which at first glance appears to
be impossible to parallelise, due to the loop carried dependency
through w:

    std::complex<double> w=std::complex<double>(1.0, 0.0);

    for (size_t j=0;j<m;j++){
      std::complex<double> t1 = w*pOut[m+j];
      std::complex<double> t2 = pOut[j]-t1;
      pOut[j] = pOut[j]+t1;
      pOut[j+m] = t2;
      w = w*wn;
    }

However, we can in fact parallelise this loop as long as
we exploit some mathematical properties and batch things
carefully. On each iteration the loop calculates w=w*wn,
so if we look at the value of w at the start of each
iteration we have:

1. j=0, w=1
2. j=1, w=1*wn
3. j=2, w=2*wn*wn

Generalising, we find that for iteration j, w=wn^j.

Hopefully it is obvious that raising something to the
power of i takes substantially less than i operations.
Try calculating (1+1e-8)^1e8 in matlab, and notice:

1. It is almost equal to _e_. Nice.
2. It clearly does not take anything like 1e8 operations to calculate.

In C++ the std::complex class supports the `std::pow` operator,
which will raise a complex number to a real scalar, which
can be used to jump ahead in the computation. In principle
we could use this property to make the loops completely
independent, but this will likely slow things down, as
powering is quite expensive (compared to one multiply). Instead we can use the idea
of _agglomeration_, which means instead of reducing the
code to the finest grain parallel tasks we can, we'll
group some of them into sequential tasks to reduce
overhead. Agglomeration is actually the same principle as the chunking
above, but now we have to think more carefully about
how to split into chunks.

So within each chunk [i,j) we need to:

1. Skip wn ahead to wn^i
2. Iterate as before over [i,j)

Possible factors of K are easy to choose in this case,
because m is always a power of two, so K can be
any power of two less than or equal to m (including K=1).
	
### Create and register a new class 

Create a new class based on `src/fast_fourier_transform.cpp`, with:
- File name: `src/your_login/fast_fourier_transform_parfor.cpp`
- Class name: `hpce::your_login::fast_fourier_transform_parfor`
- Display name: `hpce.your_login.fast_fourier_transform_parfor`

This class should be based on the sequential version, not on the
task based version, so there is only one kind of parallelism.

### Apply the loop transformation

Apply the loop transformation described above,
without introducing any parallelism, and check it works
with various values of K, via the environment variable
`HPCE_FFT_LOOP_K`.

Note that m gets smaller and smaller as it splits, so
you need to worry about m < K at the leaves of the
recursion. A simple solution is to use a guarded
version, such that if m < = K the original code is used,
and if m > K the new code is used.

As before, if `HPCE_FFT_LOOP_K` is not set, choose a sensible
default based on your analysis of the scaling with n, and/or
experiments. Though remember, it should be a sensible default
for all machines (even those with 64 cores).

7. Combine both types of parallelism
====================================

We now have two types of parallelism that we know work,
so the natural choice is to combine them together.
Create a new implementation called `fast_fourier_transform_combined`,
using the conventions for naming from before, and integrate
both forms of parallelism. This version should use either or both
of the `HPCE_FFT_LOOP_K` and `HPCE_FFT_RECURSION_K` variables, and
fall back on a default if either or both is not set.

Exploring the performance of this one is quite interesting,
particularly the interaction between the loop and recursion
chunk sizes.

8. Optimisation
===============

Our final implementation is a tweaking pass of the
parallelised version: we have introduced and exploited
two types of parallelism, but there are other optimisations
that could be done.

Create a new implementation called `fast_fourier_transform_opt`,
and apply any optimisations you think reasonable (I'm not
talking about low-level tweaks, these should be low-hanging
fruit). Particular things to consider are:

- You may wish to stop creating seperate tasks during the
  splitting, once the sub-tasks drop below a certain size.

- There is some overhead involved in the serial version
  because it is radix2 (i.e. there is a base-case where n==2).
  You may want to look at stopping sequential splitting when n==4.
  
- The timing program measures a forward and then backwards
  transform. Are there any serial bottlenecks that can be
  removed, and if so, what chunk size should be used?

To re-iterate, the aim here is not low-level optimisation and
tweaking, it is for tuning and re-organising at a high-level,
while keeping reasonably readable and portable code. By applying
these techniqes, I got about a 6 times speed-up over the original
code using a 8-core (hyper-threaded) machine for largish n.

9. Measuring execution time and scalability
===========================================

I am not going to dictate how you explore scalability,
as I don't want to over-specify, but it is something you should
naturally be doing. Particular parameters to look at are:

- Parallel overhead. How fast is the TBB version with 1 processor
  versus the serial processor?

- Number of processors versus execution time. If you double
  the processor count, is it twice as fast?
  
- The various values of K versus execution time. You would
  typically expect a bathtub curve, where K=1 is very slow
  due to parallel overhead, and K=max is slow because there
  is no parallelism, but how does it behave inbetween?

The program `time_fourier_transform` will allow you to
measure scalability against n, and to vary the number
of processors. You could copy and modify this to automatically
explore other properties, by noting that `setenv` can be
used to change an environment variable from within a
program.

Alternatively, you can exploit shell scripting, for
example, something like:

    for K="1 2 4 8 16 32"; do
        echo $K;
        export HPCE_FFT_RECURSION_K=$K;
        ./time_fourier_transform hpce.your_login.fast_fourier_transform_taskgroup
    end
    
So formally I require nothing from you to demonstrate
experiments in scaling. However, I encourage you to commit
and submit a few notes or documents about what you did,
for example some graphs or results, as I may ask you
about this coursework in the oral and it would be useful
for you to know (or show) what your did.

10. Submission
=============

Double-check your names all match up, as I'll be trying
to create your transforms both by direct instantiation,
and by pulling them out of the transform registry. Also,
don't forget that "your_login" does actually mean your
login needs to be substituted in wherever it is mentioned.

Hopefully this should all still be in a git repository, and you
will also have a private remote repository in the HPCE
organisation. "Push" your local repository to your private
remote repository, making sure all the source files have
been staged and added. (You can do a push whenever you
want as you go (it's actually a good idea), but make sure
you do one for "submission"). You may want to do a "clone" into
a seperate directory to make sure that everything has
made it.

After pushing to your private remote repository,
zip up your directory, including the .git, but excluding
any binary files (executables, objects)

I would suggest compiling and running your submission
in AWS, just to get the feel of it. A correctly written
submission should compile anywhere, with no real depencency
on the environment, but it is good to try things out.
