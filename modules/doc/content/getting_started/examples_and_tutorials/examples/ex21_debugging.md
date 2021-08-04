# Example 21: Debugging

For a more in-depth discussion of debugging with MOOSE [see the Debugging Documentation](application_development/debugging.md)

- It's inevitable: at some point in your MOOSE application development career, you will create a bug.
- Sometimes, print statements are sufficient to help you determine the cause of the error.
- For more complex bugs, a debugger can be more effective than print statements in helping to pinpoint the problem.
- Many debuggers exist: LLDB, GDB, Totalview, ddd, etc.
- It's typically best to use a debugger that is associated with your compiler, if one is available.
- Here we focus on LLDB/GDB since it is relatively simple to use and is included in the MOOSE binary redistributable package.
- A "Segmentation fault," "Segfault," or "Signal 11" error denotes a memory bug (often array access out of bounds).
- In your terminal you will see a message like:

```text
Segmentation fault: 11`
```

- A segfault is a "good" error to have, because a debugger can easily pinpoint the problem.

[](---)

# Debugging Example Problem

- This example is similar to Example 3, except that a common error has been introduced.
- In `ExampleDiffusion.h`, a `VariableValue` that should be declared as a reference is not: `const VariableValue _coupled_coef`
- Not storing this as a reference will cause a **copy** of the `VariableValue` to be made.
- That copy will never be resized, nor will it ever have values written to it.
- Attempting to access that `VariableValue` results in a segfault when running in optimized mode:

```text
Time Step  0, time = 0
                dt = 0

Time Step  1, time = 0.1
                dt = 0.1
Segmentation fault: 11
```

- We can use a debugger to help us find the problem.

[](---)

# Debug Executable

- To use a debugger with a MOOSE-based application, you must compile your application in something other than optimized mode (opt). We highly recommend debug (dbg) mode so that you'll get full line number information in your stack traces:

```text
cd ~/projects/moose/examples/ex21_debugging
METHOD=dbg make -j8
```

- You will now have a "debug version" of your application called `ex21-dbg`.
- Next, you need to run your application using either GDB (gcc) or LLDB (clang):

```text
gdb --args ./ex21-dbg -i ex21.i
```

```text
lldb -- ./ex21-dbg -i ex21.i
```

- When using either of these tools, the command line arguments to the application appear after the `--` separator.
- This will start debugger, load your executable, and leave you at the debugger command prompt.

[](---)

# Using GDB or LLDB

- At any prompt in GDB or LLDB, you can type `h` and hit enter to get help.
- We set a "breakpoint" in `MPI_Abort` so that the code pauses (maintaining the stack trace) before exiting.

```text
b MPI_Abort
```

- To run your application, type `r` and hit enter.
- If your application hits the breakpoint in `MPI_Abort` you know it has crashed.
- Type `where` (or `bt`) to see a backtrace.

```text
    frame #0: 0x0000000106b14a20 libmpi.12.dylib`MPI_Abort
    frame #1: 0x00000001000d8e78 libex21-dbg.0.dylib`MooseArray<double>::operator[](this=0x0000000108852680, i=0) const + 2200 at MooseArray.h:267
    frame #2: 0x00000001000d85ab libex21-dbg.0.dylib`ExampleDiffusion::computeQpResidual(this=0x0000000108852000) + 43 at ExampleDiffusion.C:40
    frame #3: 0x0000000100c2affb libmoose-dbg.0.dylib`Kernel::computeResidual(this=0x0000000108852000) + 443 at Kernel.C:57
.....
```

[](---)

- This backtrace shows that, in `ExampleDiffusion::computeQpResidual()` we tried to access entry 0 of a `MooseArray` with 0 entries.
- If we look at the relevant line of code, we'll see:

```C++
return _coupled_coef[_qp]*Diffusion::computeQpResidual();
```

- There is only one thing we're indexing into on that line: `_coupled_coef`.
- Therefore, we can look at how `_coupled_coef` was declared, realize that we forgot an ampersand (`&`), and fix it!

[](---)

# Example 21 Source Code

[ex21.i](https://github.com/idaholab/moose/blob/devel/examples/ex21_debugging/ex21.i)

[](---)

[ExampleDiffusion.h](https://github.com/idaholab/moose/blob/devel/examples/ex21_debugging/include/kernels/ExampleDiffusion.h)

[](---)

[ExampleDiffusion.C](https://github.com/idaholab/moose/blob/devel/examples/ex21_debugging/src/kernels/ExampleDiffusion.C)

!content pagination use_title=True
                    previous=examples/ex20_user_objects.md
