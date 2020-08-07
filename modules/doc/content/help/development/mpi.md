
# Message Passing Interface (MPI)

The Message Passing Interface (MPI) allows one to utilize multiple cores, generally to solve problems quicker than otherwise possible when using only one core. There is nothing magical about it. Not all binaries or applications are designed to make use of multi-core systems. MOOSE however, is scalable. MOOSE can benefit from using multiple cores.

## Argument Syntax

We will only be discussing the very basic syntax needed to run MOOSE in parallel.

```bash
mpiexec -n 2 <binary>
```

!alert note title=mpiexec or mpirun
On some systems, `mpiexec` is available as `mpirun`

The above will provide 2 processing cores to be utilized while running `binary`. Any binary can be executed, but not all binaries will make use of multiple cores. For example, if we instruct `echo` to be executed with MPI using *x* cores, `echo` will print whatever we instructed it to print, *x* amount of times.

```bash
$> mpiexec -n 2 echo 'hello world'
hello world
hello world
```

`echo` is not able to benefit from running in parallel (a common phrase when using more than 1 processor), to print one line of 'hello world' any quicker than if we executed it serially (a common phrase when using only 1 processor). However, it does prove MPI is working.

## Running MOOSE in Parallel

For the following example, we will use the Simple Diffusion problem which was generated when you created your application using Stork. Assuming you have already built and successfully ran your application's tests, enter your applications test directory, and run your application using mpiexec, to solve the Simple Diffusion problem:

```bash
cd ~/projects/YourAppName/test/tests/kernels/simple_diffusion
mpiexec -n 2 ../../../../yourappname-opt -i simple_diffusion.i
```

!alert note title=YourAppName
Obviously change 'YourAppName' and 'yourappname' to the name of the application you decided upon during your adventure with 'Getting Started'. The actual binary will be lower case. CamelCase will be separated with underscores.

MOOSE will output some initialization steps, which includes how many processors it will use to solve the problem:

```pre
Parallelism:
  Num Processors:          2
  Num Threads:             1
```

What's this about threads? Think of threads as 'virtual cores'. Every core has two threads. However, MPI does not make use of them. Threads is something an application must be willing to use. MOOSE, can make use of threads, but generally supplying physical cores is the better solution when solving large problems. Regardless, you can attempt to use a combination of multiple cores/threads with MOOSE by supplying the --n-threads=*num* argument:

*Assuming you still residing in `~/projects/YourAppName/test/tests/kernels/simple_diffusion`*

Just threads:

```bash
../../../../yourappname-opt --n-threads=2 -i simple_diffusion.i
```

Combination:

```bash
mpiexec -n 2 ../../../../yourappname-opt --n-threads=2 -i simple_diffusion.i
```

Okay, everytime I run the above it finishes just as fast. How can I make my machine *actually* do some work, so I know MPI/Threads is helping?

One simple way of making any problem more difficult, is to increase the refinment of the mesh (-r *#*):

```bash
mpiexec -n 2 ../../../../yourappname-opt -i simple_diffusion.i -r 5
```

If you want to time these experiments, you can either use `time`, or supply `--timing | -t` to your application arguments:

```bash
time mpiexec -n 2 ../../../../yourappname-opt -i simple_diffusion.i -r 5
# or
mpiexec -n 2 ../../../../yourappname-opt -i simple_diffusion.i -r 5 --timing
```

There is a thing as 'too many cores'... Especially with our easy-to-solve Simple Diffusion problem. More cores can mean a decrease in performance. MPI has to allocate memory for each core, as well as all the additional communication between each processor involved. So -n 28 isn't going to help:

```bash
mpiexec -n 28 ../../../../yourappname-opt -i simple_diffusion.i -r 5
```
