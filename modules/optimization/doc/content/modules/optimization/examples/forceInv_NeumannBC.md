# Force Inversion Example: Neumann Boundary Condition

## Background

The MOOSE optimization module provides a flexible framework for solving inverse optimization problems in MOOSE.  This page is part of a set of examples for different types of inverse optimization problems.

- [Theory](theory/InvOptTheory.md)
- [Examples overview](optimization/examples/index.md)
- [Example 1: Point Loads](forceInv_pointLoads.md)
- [Example 2: Neumann Boundary Condition](forceInv_NeumannBC.md)
- [Example 3: Distributed Body Load](forceInv_BodyLoad.md)
- [debuggingHelp.md]

# Example: Neumann Boundary Condition id=sec:NeumannBC

## Main-App Optimization Executioner

In this example, we will be optimizing the multi-objective function given by:

\begin{equation}\label{eq:obj}
f(\mathbf{u}, \mathbf{p}) = \frac{1}{2}\sum_{i=1}^N \left(u_i^x - \tilde{u}_i^x\right)^2 + \frac{1}{2}\sum_{j=1}^M \left(u_j^y - \tilde{u}_j^y\right)^2,
\end{equation}

where $\mathbf{u}$ represents the displacement vector, $\mathbf{p}$ denotes the parameters, $u_i^x$ and $\tilde{u}_i^x$ are the observed and target $x$-components of the displacement at the $i$-th point, and $u_j^y$ and $\tilde{u}_j^y$ are the observed and target $y$-components of the displacement at the $j$-th point.

The derivative of the objective function with respect to the parameters $\mathbf{p}$ can be expressed as:

\begin{equation}\label{eq:obj_derivative}
\frac{\partial f}{\partial \mathbf{p}} = \frac{\partial f_x}{\partial \mathbf{p}} + \frac{\partial f_y}{\partial \mathbf{p}}.
\end{equation}

In this example we will be optimizing the two forcing parameters on the right side of a cantilever beam. The forcing parameters control the force that is applied to the beam in each direction.

The main app, shown in [main_file], only receives the final objective and gradient.

!listing optimization/invOpt_mechanics/main_auto_adjoint.i  id=main_file caption=MainApp Input File

## Forward and Adjoint Problem Sub-App

The definition of the objective function in [eq:obj] allows for the gradient to be the addition of two gradients: one for the $x$-component and one for the $y$-component. The forward sub app is responsible for correctly creating both sub-gradients and combining them, as well as the objective functions, to be used by the main app. This example will only go into detail on how to calculate and combine these objectives and gradients. For more details on setting up the entire optimization process, look at [Example 1: Point Loads](forceInv_pointLoads.md).

For functions, we split the parameter vector, which has two entries, into two separate functions: one that controls the force for the $x$-component and another that controls the force for the $y$-component.

!listing optimization/invOpt_mechanics/forward_and_adjoint.i  id=f_a_func block=Functions caption=Functions

For the boundary conditions, we apply the forces as Neumann boundary conditions.

!listing optimization/invOpt_mechanics/forward_and_adjoint.i  id=f_a_bcs block=BCs caption=Boundary Conditions

For the reporters, we create two [OptimizationData](OptimizationData.md) objects that hold the observed measurement data for $x$ and $y$ individually. This Reporter will calculate the individual objective values. The locations or the number of points for $x$ and $y$ don't need to match. Also, we use a [ParsedVectorReporter](ParsedVectorReporter.md) and a [ParsedScalarReporter](ParsedScalarReporter.md) to combine the individual gradients and objective function, respectively. These are the final reporters that will be transferred to the main app.

!listing optimization/invOpt_mechanics/forward_and_adjoint.i  id=f_a_reps block=Reporters caption=Reporters

The Dirac kernels show how we apply the misfit to the adjoint variables, with the adjoint $\lambda_x$ getting the misfit from the $x$ data and the $\lambda_y$ getting the misfit from the $y$ data.

!listing optimization/invOpt_mechanics/forward_and_adjoint.i  id=f_a_dk block=DiracKernels caption=Dirac Kernels

Finally, in vector postprocessors, we show that we calculate the gradients for both components using [SideOptimizationNeumannFunctionInnerProduct](SideOptimizationNeumannFunctionInnerProduct.md) with the correct adjoint variable and function. These gradients are then combined in the reporters block as previously discussed.

!listing optimization/invOpt_mechanics/forward_and_adjoint.i  id=f_a_vpp block=VectorPostprocessors caption=Vector Postprocessors

