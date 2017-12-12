# Adding a Custom Kernel

## Overview
---
This example builds on [Example 1](examples/Example_01.md) and introduces how to create a custom [Kernel](http://www.mooseframework.org/docs/doxygen/moose/classKernel.html), which is the mechanism for adding custom physics into [MOOSE].

## Problem Statement
---

We consider the steady-state advection-diffusion equation for the 3-D domain $\Omega$ find $u$ such that

$ -\nabla \cdot \nabla u + \vec{v} \cdot \nabla u = 0$,

$u=1$ on the bottom, $u=0$ on the top and with $\nabla u \cdot \hat{n} = 0$ on the remaining boundaries. The velocity, $\vec{v}$ is a known constant (1 in the vertical (z) direction and zero otherwise).

The weak form of this equation, in inner-product notation, is given by:

$ (\nabla \phi_i, \nabla u_h) + (\vec{v} \cdot \nabla u, \phi_i)= 0 \quad \forall  \phi_i $,

where $\phi_i$ are the test functions and $u_h$ is the finite element solution.

## Create Advection Kernel
---

The advection component of the problem is defined by creating a C++ object that inherits from an exiting MOOSE object.
In general, adding a new object to MOOSE requires only two steps:

- Create a C++ object that inherits from the appropriate MOOSE object, in this case a [Kernel](http://www.mooseframework.org/docs/doxygen/moose/classKernel.html).
- Register this new object in your application, in this case `ExampleApp.C`.


## Create Object
---

- [ExampleConvection.h](https://github.com/idaholab/moose/blob/devel/examples/ex02_kernel/include/kernels/ExampleConvection.h)
- [ExampleConvection.C](https://github.com/idaholab/moose/blob/devel/examples/ex02_kernel/src/kernels/ExampleConvection.C)


## Register Object
---

Within your application, which was generated using [Stork](/stork.md), the newly created object must be registered. For this example, the application source code is `/src/base/ExampleApp.C`. To add the `Kernel` created above simply include the header and register it within the `registerObjects` method.

- [ExampleApp.C](https://github.com/idaholab/moose/blob/devel/examples/ex02_kernel/src/base/ExampleApp.C)


## Input File Syntax
---

The only difference between this example and [Example 1](examples/Example_01.md) is that the custom Kernel object created above must be included. Since this new object was registered, it is available and accessed using similar syntax to the `Diffusion` kernel. Thus, the `[Kernels]` block in the input file becomes:

!listing examples/ex02_kernel/ex02.i start=Kernels end=BCs

Note, the variable name was also changed in this example to `convected`.

## Running the Problem
---

!media media/examples/ex02_out.png width=30% float=right margin-left=1% caption=Figure 2: Example 2 Results

This example may be run using [Peacock](application_use/Peacock.md) or by running the following commands form the command line.

```
cd ~/projects/moose/examples/ex02_kernel
make -j8
./ex02-opt -i ex02.i
```

This will generate the results file, out.e, as shown in Figure 2. This file may be viewed using [Peacock](application_use/Peacock.md) or an external application that supports the Exodus II format (e.g., [Paraview](https://www.paraview.org/)).

## Complete Source Files
---

- [ex02.i](https://github.com/idaholab/moose/blob/devel/examples/ex02_kernel/ex02.i)
- [ExampleConvection.h](https://github.com/idaholab/moose/blob/devel/examples/ex02_kernel/include/kernels/ExampleConvection.h)
- [ExampleConvection.C](https://github.com/idaholab/moose/blob/devel/examples/ex02_kernel/src/kernels/ExampleConvection.C)
- [ExampleApp.C](https://github.com/idaholab/moose/blob/devel/examples/ex02_kernel/src/base/ExampleApp.C)
- [ex02_oversample.i](https://github.com/idaholab/moose/blob/devel/examples/ex02_kernel/ex02_oversample.i)
