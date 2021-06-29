# Debugging

!---

A debugger is often more effective than print statements in helping to find bugs

Many debuggers exist: LLDB, GDB, Totalview, ddd, Intel Debugger, etc.

Typically it is best to use a debugger that is associated with your compiler

Here LLDB/GDB are used, both are simple to use and are included in the MOOSE package

!---

## Segmentation Fault

A "Segmentation fault," "Segfault," or "Signal 11" error denotes a memory bug (often array access out
of bounds).

```bash
Segmentation fault: 11
```

A segfault is a "good" error to have, because a debugger can easily pinpoint the problem.

!---

## Example 21: Input File

!listing examples/ex21_debugging/ex21.i

!---

## Example 21: Run Input File

```bash
cd ~/projects/moose/examples/ex21_debugging
make -j12
./ex21-opt -i ex21.i

Time Step  0, time = 0
                dt = 0

Time Step  1, time = 0.1
                dt = 0.1
Segmentation fault: 11
```

!---

## Debug Compile

To use a debugger with a MOOSE-based application, it must be compiled in something
other than optimized mode (opt); debug (dbg) mode is recommended because it will produce full line
number information in stack traces:

```text
cd ~/projects/moose/examples/ex21_debugging
METHOD=dbg make -j12
```

This will create a "debug version" of and application: `ex21-dbg`

!---

## Running Debugger

The compiled debug application can be executed using either GDB (gcc) or LLDB (clang):

```bash
gdb --args ./ex21-dbg -i ex21.i
```

```bash
lldb -- ./ex21-dbg -i ex21.i
```

These commands will start debugger, load the executable, and open the debugger command prompt

!---

## Using GDB or LLDB

At any prompt in GDB or LLDB, you can type `h` and hit enter to get help

1. Set a "breakpoint" in `MPI_Abort` so that the code pauses (maintaining the stack trace)

   ```bash
   (lldb) b MPI_Abort
   Breakpoint 1: where = libmpi.12.dylib`MPI_Abort, address = 0x000000010b18f460
   ```

2. Run the application, type `r` and hit enter, the application will hit the breakpoint.

   ```bash
   (lldb) r
   Process 77675 launched: './ex21-dbg' (x86_64)
   ```

3. When the application stops, get the backtrace

   ```bash
   (lldb) bt
    * thread #1, queue = 'com.apple.main-thread', stop reason = breakpoint 1.1
      * frame #0: 0x000000010b18f460 libmpi.12.dylib`MPI_Abort
        frame #1: 0x00000001000e5f8c libex21-dbg.0.dylib`MooseArray<double>::operator[](this=0x0000000112919388, i=0) const at MooseArray.h:276
        frame #2: 0x00000001000e580b libex21-dbg.0.dylib`ExampleDiffusion::computeQpResidual(this=0x0000000112918a18) at ExampleDiffusion.C:37
        frame #3: 0x0000000100486b99 libmoose-dbg.0.dylib`Kernel::computeResidual(this=0x0000000112918a18) at Kernel.C:99
   ```

!---

The backtrace shows that in `ExampleDiffusion::computeQpResidual()` an attempt was made to access
entry 0 of a `MooseArray` with 0 entries.


```C++
return _coupled_coef[_qp] * Diffusion::computeQpResidual();
```

There is only one item being indexed on that line: `_coupled_coef`; therefore, consider how
`_coupled_coef` was declared

!---

## Bug

In `ExampleDiffusion.h`, the member variable `_coupled_coef` is a `VariableValue` that should be
declared as a reference:

```bash
const VariableValue _coupled_coef;
```

Not storing this as a reference will cause a +copy+ of the `VariableValue` to be made during
construction. This copy will never be resized, nor will it ever have values written to it.

!---

## ExampleDiffusion.h

!listing examples/ex21_debugging/include/kernels/ExampleDiffusion.h

!---

## ExampleDiffusion.C

!listing examples/ex21_debugging/src/kernels/ExampleDiffusion.C
