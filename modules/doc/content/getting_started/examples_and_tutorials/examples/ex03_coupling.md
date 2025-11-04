# Example 3 : Multiphysics coupling

## Problem statement

This problem considers a coupled systems of equations on a 3-D domain $\Omega$ : find $u$ and $v$
such that

\begin{equation}
\begin{aligned}
-\nabla \cdot \nabla u + \nabla v \cdot \nabla u = 0 \\
-\nabla \cdot \nabla v = 0
\end{aligned}
\end{equation}

where $u=v=0$ on the top boundary and $u=2$ and $v=1$ on the bottom boundary. The remaining
boundaries are natural boundaries:

\begin{equation}
\begin{aligned}
\nabla u \cdot \hat{n} = 0 \\
\nabla v \cdot \hat{n} = 0
\end{aligned}
\end{equation}

The domain, $\Omega$, is the same as utilized in Example 2.  The weak form of this equation, in
inner-product notation, is given by:

\begin{equation}
\begin{aligned}
(\nabla u_h, \nabla \phi_i) + (\nabla v \cdot \nabla u, \phi_i)= 0 \quad \forall  \phi_i \\
(\nabla\vec{v}, \nabla\phi_i)= 0 \quad \forall  \phi_i
\end{aligned}
\end{equation}

where $\phi_i$ are the test functions and $u_h$ and $v_h$ are the finite element solutions.

## Create Convection Kernel

The convection component of the problem requires `Kernel` object just as described in Example 2
with one small addition - the `Kernel` will utilize a coupled variable rather than a known
constant.  "ExampleConvection.h" needs one new member variable to store the gradient of the
coupled variable:

!listing examples/ex03_coupling/include/kernels/ExampleConvection.h start=private end=}

The source file "ExampleConvection.C" also includes a new parameter that defines the variable to
couple into its kernel. Additionally, the `computeQpResidual` and `computeQpJacobian` functions in
the source file now utilize the coupled value to compute the desired residuals and jacobians
respectively:

!listing examples/ex03_coupling/src/kernels/ExampleConvection.C start=#include end=$ max-height=10000px

## Input File Syntax

First, the mesh is defined by loading a file "mug.e".

!listing examples/ex03_coupling/ex03.i block=Mesh

Then, the two variables are defined: "diffused" and "convected", which refer to $u$ and $v$
from the problem statement, respectively. Both variables in this case are assigned to utilize
linear Lagrange shape functions, but they could each use different shape functions and/or orders.

!listing examples/ex03_coupling/ex03.i block=Variables

The problem requires three `Kernels`, two `Diffusion` `Kernels`, one for each of the variables and
the `ExampleConvection` `Kernel` created above. It is important to point out that for the two
`Diffusion` terms, the same code is utilized; two instances of the C++ object are created to
application of the code to two variables.  Additionally, the actual coupling of the equations
takes place in the `ExampleConvection` object. The `some_variable` input parameter was created in
the `ExampleConvection` `Kernel` and here is assigned to utilize the `diffused` variable.

!listing examples/ex03_coupling/ex03.i block=Kernels

For the given problem, each of the variables has a `DirichletBC` applied at the top and bottom.
This is done in the input file as follows.

!listing examples/ex03_coupling/ex03.i block=BCs max-height=10000px

Finally, the `Executioner` block is setup for solving the problem an the `Outputs` are set for
viewing the results.

!listing examples/ex03_coupling/ex03.i start=Executioner end=$

## Running the Problem

This example may be run using Peacock or by running the following commands form the command line.

```
cd ~/projects/moose/examples/ex03_coupling
make -j8
./ex03-opt -i ex03.i
```

This will generate the results file, out.e, as shown in Figure 1 and 2. This file may be viewed
using Peacock or an external application that supports the Exodus format (e.g., Paraview).

!media large_media/examples/ex03_out_diffused.png
       caption=Figure 1: example 3 Results, "diffused variable"
       style=width:40%;display:inline-flex;margin-left:7%;

!media large_media/examples/ex03_out_convected.png
       caption=Figure 2: example 3 Results, "convected variable"
       style=width:40%;display:inline-flex;margin-left:7%;

# 1D exact solution

 A simplified 1D analog of this problem is given as follows, where $u(0)=0$ and $u(1)=1$:

\begin{equation}
-\epsilon \frac{d^2 u}{d x^2} + \frac{d u}{d x} = 0
\end{equation}

The exact solution to this problem is:

\begin{equation}
u = \frac{\exp\left(\frac{x}{\epsilon}\right) - 1}{\exp\left(\frac{1}{\epsilon}\right) - 1}
\end{equation}

## Complete Source Files

- [examples/ex03_coupling/ex03.i]
- [examples/ex03_coupling/include/kernels/ExampleConvection.h]
- [examples/ex03_coupling/src/kernels/ExampleConvection.C]
- [examples/ex03_coupling/src/base/ExampleApp.C]

!content pagination use_title=True
                    previous=examples/ex02_kernel.md
                    next=examples/ex04_bcs.md
