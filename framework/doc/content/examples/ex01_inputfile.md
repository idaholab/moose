


  # Problem Statement

- This example briefly describes the creation of a basic input file and the six required components for utilizing [`MOOSE`][moose] for solving a problem.
- We consider the steady-state diffusion equation on the 3-D domain $$ \Omega $$: find $$u$$ such that
$$$ -\nabla \cdot \nabla u = 0 \in \Omega,$$$
$$u = 1$$ on the bottom, $$u = 0$$ on the top and with $$\nabla u \cdot \hat{n} = 0$$ on the remaining boundaries.

- The weak form ([see Finite Elements: The MOOSE Way](http://mooseframework.org/wiki/MooseTraining/FEM)) of this equation, in inner-product notation, is given by:
$$$ (\nabla \phi_i, \nabla u_h) = 0 \quad \forall  \phi_i, $$$
where $$\phi_i$$ are the test functions and $$u_h$$ is the finite element solution.

[](---)
# Input File Syntax
- A basic moose input file requires six parts:
    - Mesh
    - Variables
    - Kernels
    - BCs
    - Executioner
    - Outputs
- Documentation for these systems and all MOOSE systems is included on the wiki: [http://mooseframework.org/wiki/MooseSystems](http://mooseframework.org/wiki/MooseSystems)
- A general overview of the basic MOOSE input file syntax is provided on the wiki: [http://mooseframework.org/wiki/MooseTraining/InputFile](http://mooseframework.org/wiki/MooseTraining/InputFile)

[](---)
## Mesh

- The domain for the problem is created with the "Mesh" block in the input file. 
- Here the mesh, the mesh is read from the file mug.e.
```text
[Mesh]
  file = 'mug.e'
[]
```

!media media/examples/mug_mesh.png
       caption=mug.e mesh file
       style=width:50%;


[](---)

## Variables

- In this simple problem, a single variable, 'diffused,' is defined, which represents $$u$$ from the continuous problem. 
- The 'diffused' variable is approximated with linear Lagrange shape functions.

```text
[Variables]
  [./diffused]
    order = FIRST
    family = LAGRANGE
  [../]
[]
```


[](---)
## Kernels
- The weak form of problem statement is represented by a Diffusion [`Kernel`][kernel] object.  
- In general, users write their own Kernels and store them in their own MOOSE-based application, but in this case the Diffusion Kernel is already defined in MOOSE.

- Within the input file, to invoke the use of this object a sub-block is defined, named "diff", that utilizes the [`Diffusion`][diffusion] [`Kernel`][kernel] on the previously defined variable "diffused".

```text
[Kernels]
  [./diff]
    type = Diffusion
    variable = diffused
  [../]
[]
```

[](---)
## Boundary Conditions (BCs)
- Boundary conditions are defined in a similar manner as [`Kernels`][kernel]. 
- For the current problem two Dirichlet boundary conditions are required, again an object for this type of boundary is already defined in a C++ object within MOOSE: [`DirichletBC`][bc]. 
- In the input file the two boundary conditions are applied utilizing a single C++ object as follows.

```text
[BCs]
  [./bottom]
    type = DirichletBC
    variable = diffused
    boundary = 'bottom'
    value = 1
  [../]
  [./top]
    type = DirichletBC
    variable = diffused
    boundary = 'top'
    value = 0
  [../]
[]
```

- Within each of the two sub-blocks, named "top" and "bottom" by the user, the boundary conditions are linked to the associated variable ("variable = diffused") and boundary. 
- The supplied mesh file, mug.e, prescribes and labels the boundaries "top" and "bottom", these are often numbers depending on how your mesh file was generated.

- Note, the Neumann boundary condition for this problem is automatically satisfied, thus there is no need to define it. However, non-zero Neumann conditions as well as many others may be defined using existing MOOSE objects (e.g., [`NeumannBC`][bc2]) or using custom boundary conditions derived from the existing objects within MOOSE.

[](---)
## Executioner
- The type of problem to solve and the method for solving is defined within the [`Executioner`][exec] block. 
- For this problem the type is [`Steady`][steady] and the method for solving is the default, Preconditioned Jacobain Free Newton Krylov.

```text
[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]
```

[](---)
## Outputs
- Here two types of outputs are enabled: output to the screen (console) and output to an Exodus II file (exodus). 
- Setting the "file_base" parameter is optional, in this example it forces the output file to be named "out.e" ("e" is the extension used for the Exodus II format).

```text
[Outputs]
  file_base = out
  screen = true
  exodus = true
[]
```

[](---)
# Running the Problem

- This example may be run using [Peacock][gui] or by running the following commands form the command line.
```bash
cd ~/projects/moose/examples/ex01_inputfile
make -j8
./ex01-opt -i ex01.i
```

- This will generate the results file, out.e, as shown in Figure 2. This file may be viewed using [Peacock][gui] or an external application that supports the Exodus II format (e.g., Paraview).

!media media/examples/ex01_results.png
       caption= Example 1 Results
       style=width:50%;


[](---)
# Complete Source Files
[ex01.i](https://github.com/idaholab/moose/blob/devel/examples/ex01_inputfile/ex01.i)

[main.C](https://github.com/idaholab/moose/blob/devel/examples/ex01_inputfile/src/main.C)

[ExampleApp.h](https://github.com/idaholab/moose/blob/devel/examples/ex01_inputfile/include/base/ExampleApp.h)

[ExampleApp.C](https://github.com/idaholab/moose/blob/devel/examples/ex01_inputfile/src/base/ExampleApp.C)

[Diffusion.h](https://github.com/idaholab/moose/blob/devel/framework/include/kernels/Diffusion.h)


[Diffusion.C](https://github.com/idaholab/moose/blob/devel/framework/src/kernels/Diffusion.C)


[](---)
# Next Steps

- Diffusion kernel is the only "physics" in the MOOSE framework
    * A large set of physics is included in the MOOSE [modules](http://mooseframework.org/wiki/PhysicsModules/)
- In order to implement your own physics, you must understand three things:
    * [Finite Element Methods](http://mooseframework.org/wiki/MooseTraining/FEM/) and the generation of a "weak" form
    * [C++](http://mooseframework.org/wiki/MooseTraining/CPP/) and object-oriented design
    * [The Anatomy of a MOOSE Object](http://mooseframework.org/wiki/MooseTraining/MooseObject/)

 [gui]: http://mooseframework.org/wiki/Peacock 
 [moose]: https://mooseframework.org
 [kernel]: http://www.mooseframework.org/docs/doxygen/moose/classKernel.html
 [diffusion]: http://www.mooseframework.org/docs/doxygen/moose/classDiffusion.html
 [bc]: http://www.mooseframework.org/docs/doxygen/moose/classDirichletBC.html
 [bc2]: http://www.mooseframework.org/docs/doxygen/moose/classNuemannBC.html
 [exec]: http://www.mooseframework.org/docs/doxygen/moose/classExecutioner.html
 [steady]: http://www.mooseframework.org/docs/doxygen/moose/classSteady.html
