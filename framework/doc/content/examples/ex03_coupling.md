# Example 3 : Multiphysics coupling

[](---)

## Problem statement

This problem considers a coupled systems of equations on a 3-D domain $\Omega$ : find $u$ and $v$ such that 
$-\nabla \cdot \nabla u + \nabla\vec{v} \cdot \nabla u = 0$ and

$-\nabla \cdot \nabla v = 0$,

where $u=v=0$ on the top boundary and $u=2$ and $v=1$ on the bottom boundary. The remaining boundaries are natural boundaries: $\nabla u \cdot \hat{n} = 0$ and $\nabla v \cdot \hat{n} = 0$. The domain, $\Omega$, is the same as utilized in Example 2.

The weak form of this equation, in inner-product notation, is given by:

$(\nabla u_h, \nabla \phi_i) + (\nabla\vec{v} \cdot \nabla u, \phi_i)= 0 \quad \forall  \phi_i$ and

$(\nabla\vec{v}, \nabla\phi_i)= 0 \quad \forall  \phi_i$,

where $\phi_i$ are the test functions and $u_h$ and $v_h$ are the finite element solutions.

[](---)

## Create Convection Kernel

The convection component of the problem requires the creation of a a new `Kernel`, as described in Example 02. Here the `Kernel` must utilize a coupled variable rather than a known constant. 

The header for this object, "ExampleConvection.h" is little changed from the previous example, with one exception, a member is defined to store the gradient of the coupled variable:

[ExampleConvection.h](https://github.com/idaholab/moose/blob/devel/examples/ex03_coupling/include/kernels/ExampleConvection.h)

The source filem "ExampleConvection.C", after including the header, defines the parameters for the kernel. This definition includes adding a parameter that defines the variable to couple into tis kernel.

[ExampleConvection.C](https://github.com/idaholab/moose/blob/devel/examples/ex03_coupling/src/kernels/ExampleConvection.C)

Finally, the `computeQpResiduals` and `computeQpJacobian` then utilize the coupled value to compute the desired residuals and jacobians.

```cpp
Real ExampleConvection::computeQpResidual()
{
  return _test[_i][_qp]*(_grad_some_variable[_qp]*_grad_u[_qp]);
}
```

```cpp
Real ExampleConvection::computeQpJacobian()
{
  return _test[_i][_qp]*(_grad_some_variable[_qp]*_grad_phi[_j][_qp]);
}
```

 

## Register Kernel

As done in Example 2, the newly created object must be registered. This is accomplished by including the "Convection.h" file and the following in `ExampleApp::registerObjects` method of `src/base/ExampleApp.C` within the example 3 directory of MOOSE (`examples/ex03_coupling`).

```cpp
registerKernel(ExampleConvection); 
```

[](---)

## Input File Syntax

First, the mesh is defined by loading a file "mug.e".

```puppet
[Mesh]
  file = mug.e
[]
```

Then, the two variables are defined: "diffused" and "convected", which refer to $$u$$ and $$v$$ from the problem statement, respectively. Both variables in this case are assigned to utilize linear Lagrange shape functions, but they could each use different shape functions and/or orders.

```puppet
[Variables]
  [./convected]
    order = FIRST
    family = LAGRANGE
  [../]
  [./diffused]
    order = FIRST
    family = LAGRANGE
  [../]
[]
```

The problem requires three `Kernels`, two `Diffusion` `Kernels`, one for each of the variables and the `ExampleConvection` `Kernel` created above. It is important to point out that for the two `Diffusion` terms, the same code is utilized; two instances of the C++ object are created to application of the code to two variables.  Additionally, the actual coupling of the equations takes place in the `ExampleConvection` object. The `some_variable` input parameter was created in the `ExampleConvection` `Kernel` and here is assigned to utilize the `diffused` variable.

```puppet
[Kernels]
  [./diff_convected]
    type = Diffusion
    variable = convected
  [../]
  [./conv]
    type = ExampleConvection
    variable = convected
    some_variable = diffused #Couple variable into the convection kernel
  [../]
  [./diff_diffused]
    type = Diffusion
    variable = diffused
  [../]
[]
```

For the given problem, each of the variables has a `DirichletBC` applied at the top and bottom. This is done in the input file as follows.

```puppet
[BCs]
  [./bottom_convected]
    type = DirichletBC
    variable = convected
    boundary = 'bottom'
    value = 1
  [../]
  [./top_convected]
    type = DirichletBC
    variable = convected
    boundary = 'top'
    value = 0
  [../]
  [./bottom_diffused]
    type = DirichletBC
    variable = diffused
    boundary = 'bottom'
    value = 2
  [../]
  [./top_diffused]
    type = DirichletBC
    variable = diffused
    boundary = 'top'
    value = 0
  [../]
[]
```

Finally, the `Executioner` block is setup for solving the problem an the `Outputs` are set for viewing the results.

```puppet
[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]
[Outputs]
  file_base = out
  exodus = true
  [./console]
    type = Console
    perf_log = true
    linear_residuals = true
  [../]
[]
```

[](---)

## Running the Problem

This example may be run using Peacock or by running the following commands form the command line.

```
cd ~/projects/moose/examples/ex03_coupling
make -j8
./ex03-opt -i ex03.i
```

This will generate the results file, out.e, as shown in Figure 1 and 2. This file may be viewed using Peacock or an external application that supports the Exodus II format (e.g., Paraview).

!media large_media/examples/ex03_out_diffused.png
       caption=Example 03 Results, "diffused variable"
       style=width:50%;

!media large_media/examples/ex03_out_convected.png
       caption=Example 03 Results, "convected variable"
       style=width:50%;


# 1D exact solution

 A simplified 1D analog of this problem is given as follows, where $u(0)=0$ and $u(1)=1$:
$-\epsilon \frac{d^2 u}{d x^2} + \frac{d u}{d x} = 0$

The exact solution to this problem is
$u = \frac{\exp\left(\frac{x}{\epsilon}\right) - 1}{\exp\left(\frac{1}{\epsilon}\right) - 1}$


[](---)

## Complete Source Files

[ex03.i](https://github.com/idaholab/moose/blob/devel/examples/ex03_coupling/ex03.i)      

[ExampleConvection.h](https://github.com/idaholab/moose/blob/devel/examples/ex03_coupling/include/kernels/ExampleConvection.h)

[ExampleConvection.C](https://github.com/idaholab/moose/tree/devel/examples/ex03_coupling/src/kernels/ExampleConvection.C)

[ExampleApp.C](https://github.com/idaholab/moose/blob/devel/examples/ex03_coupling/src/base/ExampleApp.C)


