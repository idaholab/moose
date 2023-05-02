# Example 06 : Transient Analysis

## Problem Statement

We consider the transient diffusion equation on the 3D domain $\Omega$: find $u$ such that
$-\nabla \cdot \nabla u = 20\frac{\partial u}{\partial t} \in \Omega$, $u = 0$ on the bottom, $u =
1$ on the top and with $\nabla u \cdot \hat{n} = 0$ on the remaining boundaries.  The initial
condition is $u(t_0) = 0$ everywhere except on the top boundary where $u = 1$.

The weak form of this equation, in inner-product notation, is given by: $\nabla \phi_i, \nabla u_h
= \phi_i, 20 \frac{\partial u_h}{\partial t} \quad \forall  \phi_i$, where $\phi_i$ are the
test functions and $u_h$ is the finite element solution.

## Constructing the Problem

First, we need a transient term to for our residual.  We can create a time-dependent residual term
by inheriting from the [`TimeDerivative`](/TimeDerivative.md) class.  For this example, we create
a constant-scalable time-dependent residual along the lines of kernels created in the earlier
examples:

!listing examples/ex06_transient/src/kernels/ExampleTimeDerivative.C start=#include end=$ max-height=10000

The input file for this is very similar to the simple diffusion input file from example 1.
We need to add the time derivative residual contribution:

!listing examples/ex06_transient/ex06.i block=Kernels

And finally, we need to specify some transient related parameters:

!listing examples/ex06_transient/ex06.i block=Executioner

There are many more options available that are described in the [Transient Executioner documentation](/executioners/Transient.md).
It is also common to use more sophisticated ways for
time-stepping through simulations. [Example 16](examples/ex16_timestepper.md) goes over some of
this and details about specific time stepping schemes are provided in the
[Timestepper System documentation](syntax/Executioner/TimeSteppers/index.md).

## Outputs

Here are solution snapshots from the beginning and end times from running `ex06-opt -i ex06.i`:

!media large_media/examples/ex6-1-out.png
       style=width:47%;display:inline-flex;

!media large_media/examples/ex6-2-out.png
       style=width:47%;display:inline-flex;margin-left:3%

## Complete Source Files

- [examples/ex06_transient/ex06.i]
- [framework/include/kernels/Diffusion.h]
- [framework/src/kernels/Diffusion.C]
- [examples/ex06_transient/include/kernels/ExampleTimeDerivative.h]
- [examples/ex06_transient/src/kernels/ExampleTimeDerivative.C]
- [examples/ex06_transient/src/base/ExampleApp.C]

!content pagination use_title=True
                    previous=examples/ex05_amr.md
                    next=examples/ex07_ics.md
