# Example 1 : As Simple as it Gets

## Problem Statement

This example briefly describes the creation of a basic input file and the six required sections
for utilizing MOOSE for solving a problem.

We consider the steady-state diffusion equation on the 3D domain $\Omega$: find $u$ such that
$-\nabla \cdot \nabla u = 0 \in \Omega$, $u = 1$ on the bottom, $u = 0$ on the top and with
$\nabla u \cdot \hat{n} = 0$ on the remaining boundaries.

The weak form ([see Finite Elements Principles](finite_element_concepts/fem_principles.md)) of
this equation, in inner-product notation, is given by: $\nabla \phi_i, \nabla u_h = 0 \quad
\forall  \phi_i$, where $\phi_i$ are the test functions and $u_h$ is the finite element solution.

## Input File Syntax

A basic moose input file requires six parts:

-  Mesh
-  Variables
-  Kernels
-  BCs
-  Executioner
-  Outputs

### Mesh

The domain for the problem is created with the `Mesh` block in the input file.  Here the mesh is
read from the file `mug.e` which is a relative path starting in the same directory as the input
file itself:


```text
[Mesh]
  file = 'mug.e'
[]
```

<br>

!media large_media/examples/mug_mesh.png
       caption=mug.e mesh file
       style=width:40%;display:block;margin-left:auto;margin-right:auto;

### Variables

In this simple problem, a single variable, 'diffused,' is defined, which represents $$u$$ from the
continuous problem.  The 'diffused' variable is approximated with linear Lagrange shape functions.

!listing examples/ex01_inputfile/ex01.i block=Variables

### Kernels

The weak form of the problem statement is represented by a `Diffusion` Kernel object.  In general,
users write custom Kernels that reside in their own MOOSE-based application(s), but in this case
the `Diffusion` Kernel is already defined in MOOSE.  To use a particular Kernel object an input
file sub-section is defined, labeled "diff" (this is an arbitrary user-defined name), that
utilizes the `Diffusion` `Kernel` and acts on the previously defined variable "diffused".

!listing examples/ex01_inputfile/ex01.i block=Kernels

### Boundary Conditions (BCs)

Boundary conditions are defined in a manner similar to `Kernels`.  For this problem two Dirichlet
boundary conditions are required.  In the input file the two boundary conditions are specified
each using the `DirichletBC` object provided by MOOSE.

!listing examples/ex01_inputfile/ex01.i block=BCs

Within each of the two sub-section, named "top" and "bottom" by the user, the boundary conditions
are linked to their associated variable(s) (i.e. "diffused" in this case) and boundaries.  In this
case, the mesh file `mug.e` defines labeled the boundaries "top" and "bottom" which we can refer
to. Boundaries will also often be specified by number IDs - depending on how your mesh file was
generated.

Note, the Neumann boundary conditions for this problem (on the left and right sides) are satisfied
implicitly and are not necessary for us to define. However, for non-zero Neumann or other boundary
conditions many built-in objects are provided by MOOSE (e.g., `NeumannBC`). You can also create
custom boundary conditions derived from the existing objects within MOOSE.

### Executioner

The type of problem to solve and the method for solving it is defined within the `Executioner`
block.  This problem is steady-state and will use the `Steady` Executioner and will use the
default solving method Preconditioned Jacobain Free Newton Krylov.

!listing examples/ex01_inputfile/ex01.i block=Executioner

### Outputs

Here two types of outputs are enabled: output to the screen (console) and output to an Exodus II
file (exodus).  Setting the "file_base" parameter is optional, in this example it forces the
output file to be named "out.e" ("e" is the extension used for the Exodus II format).

!listing examples/ex01_inputfile/ex01.i block=Outputs

## Running the Problem

This example may be run using Peacock or by running the following commands form the command line.

```bash
cd ~/projects/moose/examples/ex01_inputfile
make -j8
./ex01-opt -i ex01.i
```

This will generate the results file, out.e, as shown in [example-1-results]. This file may be viewed using
Peacock or an external application that supports the Exodus II format (e.g., Paraview).

<br>

!media large_media/examples/ex01_results.png
       id=example-1-results
       caption= Example 1 Results
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

## Next Steps

Although the Diffusion kernel is the only "physics" in the MOOSE framework, a large set of physics
is included in the MOOSE [modules](http://mooseframework.org/wiki/PhysicsModules/).  In order to
implement your own physics, you will need to understand the following:

- [Finite Element Methods](http://mooseframework.org/wiki/MooseTraining/FEM/) and the generating
  the "weak" form of for PDEs
- [C++](help/c++/index.md) and object-oriented design
- [The Anatomy of a MOOSE Object](http://mooseframework.org/wiki/MooseTraining/MooseObject/)
- Check out more [examples](examples_and_tutorials/index.md#examples)

## Complete Source Files

- [ex01.i](https://github.com/idaholab/moose/blob/devel/examples/ex01_inputfile/ex01.i)
- [main.C](https://github.com/idaholab/moose/blob/devel/examples/ex01_inputfile/src/main.C)
- [ExampleApp.h](https://github.com/idaholab/moose/blob/devel/examples/ex01_inputfile/include/base/ExampleApp.h)
- [ExampleApp.C](https://github.com/idaholab/moose/blob/devel/examples/ex01_inputfile/src/base/ExampleApp.C)
- [Diffusion.h](https://github.com/idaholab/moose/blob/devel/framework/include/kernels/Diffusion.h)
- [Diffusion.C](https://github.com/idaholab/moose/blob/devel/framework/src/kernels/Diffusion.C)

!content pagination use_title=True
                    next=examples/ex02_kernel.md
