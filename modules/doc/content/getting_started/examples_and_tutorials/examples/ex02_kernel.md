# Example 2: Adding a Custom Kernel

## Overview

This example builds on Example 1 and introduces how to create a custom `Kernel`, which is the
mechanism for adding your physics into `MOOSE`.  You can either follow along with the files in
`examples` directory in the MOOSE repository or you can create your own app as explained
[here](getting_started/new_users.md#create-an-app). When you are ready to build custom physics and
code for you own physics/problems, you should work in your own MOOSE-based application.

## Problem Statement

We consider the steady-state advection-diffusion equation for the 3-D domain $\Omega$ shown in
Fig. 1: find $u$ such that $-\nabla \cdot \nabla u + \vec{v} \cdot \nabla u = 0$, $u=1$ on the
bottom, $u=0$ on the top and with $\nabla u \cdot \hat{n} = 0$ on the remaining boundaries. The
velocity, $\vec{v}$ is a known constant (1 in the vertical z-direction and zero otherwise).

The weak form of this equation, in inner-product notation, is given by:

$(\nabla \phi_i, \nabla u_h) + (\vec{v} \cdot \nabla u, \phi_i)= 0 \quad \forall  \phi_i$,

where $\phi_i$ are the test functions and $u_h$ is the finite element solution.

## Create an Advection Kernel

The advection component of the problem is defined by creating a C++ object that inherits from an
existing MOOSE object.

In general, adding a new object to MOOSE requires creating a C++ object that inherits from the
appropriate MOOSE object, in this case a `Kernel`. The following files in this example demonstrate
this:

- [examples/ex02_kernel/include/kernels/ExampleConvection.h]
- [examples/ex02_kernel/src/kernels/ExampleConvection.C]

In addition to creating the new C++ class, newly created object classes must be "registered" in
their `.C` files.  C++ comments inside the ExampleConvection header and implementation files
further explain some of the important details.

## Input File Syntax

The only difference between this example and Example 1 is that the custom Kernel object created
above must be included. Since this new object was registered, it is available and accessed using
similar syntax to the `Diffusion` kernel. Thus, the `[Kernels]` block in the input file becomes:

```puppet
[Kernels]
  [./diff]
    type = Diffusion
    variable = convected
  [../]
  [./conv]
    type = ExampleConvection
    variable = convected
    velocity = '0.0 0.0 1.0'
  [../]
[]
```

Note, the variable name was also changed in this example to `convected`.

## Running the Problem

This example may be run using Peacock or by running the following commands form the command line.

```
cd ~/projects/moose/examples/ex02_kernel
make -j8
./ex02-opt -i ex02.i
```

This will generate the results file, out.e, as shown in Figure 2. This file may be viewed using
Peacock or an external application that supports the Exodus format (e.g., Paraview).

<br>

!media large_media/examples/ex02_out.png
       caption=Example 02 Results
       style=width:50%;display:block;margin-left:auto;margin-right:auto;


## Complete Source Files

- [examples/ex02_kernel/ex02.i]
- [examples/ex02_kernel/include/kernels/ExampleConvection.h]
- [examples/ex02_kernel/src/kernels/ExampleConvection.C]
- [examples/ex02_kernel/src/base/ExampleApp.C]
- [examples/ex02_kernel/ex02_oversample.i]

!content pagination use_title=True
                    previous=examples/ex01_inputfile.md
                    next=examples/ex03_coupling.md
