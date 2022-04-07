# RandomInterface

MOOSE currently distributes a high-quality efficient Pseudo Random Number Generator package (mtwist)
that is stable across different machine architectures. This random number generator is tied into
MOOSE's random number generator system that can generate consistent spatial random number fields as
parallel discretization changes (e.g. the number of threads/processors does not impact generated
fields). The random number interface is very straightforward to use.

!alert note
MOOSE does not currently use a forwardable PRNG - this would be a nice enhancement to the framework
and would dramatically improve the performance of random number generators on distributed meshes and
at very large scales.

## Random Number Fields

The objects in MOOSE that produce fields can directly call `getRandomLong()` or `getRandomReal()` to
access the parallel stable random number objects in MOOSE. Generally, the only user parameter that
needs to be called or set is `setRandomResetFrequency()`.  This method controls how often the
individual generators are reset. That is when they are told to replay a set of numbers.  Typically,
if the random field is used in a kernel, boundary condition, or any other object that impacts the
Residual statement, you don't want to be producing new random numbers with each linear iteration as
this can impact the convergence of the solver.  You can avoid this problem If you "reset" the random
number generators every "linear" iteration, meaning that the same random numbers are replayed each
and every linear iteration. Other options are "nonlinear" and "initial" (the latter option is
effectively no resetting).
