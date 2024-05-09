# Constrained Optimization Example: Shape Optimization

## Problem Description

In this problem, we focus on an annular (ring-shaped) domain subjected to
thermal conditions. The domain is defined by two concentric circles with an
inner and outer radii. The physical problem is heat conduction governed by the
heat equation with convective flux type boundary conditions at the inner radius
and an insulated outer radius. The convective heat transfer coefficient is
dependent on the inner radius. A constant source term is distributed throughout
the domain.

The optimization goal is to find the inner and outer radii that minimize the
maximum temperature within the domain while maintaining a fixed volume of the
annulus. In other words, the design variables are the inner and outer radii,
and the objective function is the maximum temperature within the annulus. This
is a constrained optimization problem because the volume of the annulus needs to
remain constant during the optimization process.


# Example: Shape Optimization

The MOOSE Optimization module provides a flexible framework for solving
optimization problems in MOOSE. Shape
optimization is a branch of optimization focused on finding the most efficient,
effective, or optimal shape for a given system, subject to certain constraints.
The goal is usually to minimize or maximize some performance criterion, such as
material usage, weight, aerodynamic drag, or structural stress, or as in this
case the minimizing the maximum temperature. This page will demonstrate constrained shape
optimization of an annulus while satisfying an equality volume constraint.

For constrained optimization using
[TAO](https://petsc.org/release/docs/manual/tao/), MOOSE only supports
interfacing with the
 [TAOALMM](https://petsc.org/release/manualpages/Tao/TAOALMM/) algorithm. The
 [TAOALMM](https://petsc.org/release/manualpages/Tao/TAOALMM/)  lets user to
 solve problems in the form below:

\begin{equation}
\begin{aligned}
\min_x \quad &  f(x) \\
\textit{s.t.} \quad & g(x) = 0 \\
\quad & h(x) \geq 0  \\
\quad & l \leq x \leq u \\
\end{aligned}
\end{equation}

where

- Objective function: $f(x)$
- Equality constraints: $g(x) = 0$
- Inequality constraints: $h(x) \geq 0$
- Bounds : $l \leq x \leq u$

To solve this problem we use the [MultiApps](MultiApps/index.md) system. For
this problem we have a main app that will perform all the optimization and
interfacing with [TAO](https://petsc.org/release/docs/manual/tao/), while the
forward app will solve the physics and calculate the objective and equality
constraints. These values will then be sent using
[Transfers System](Transfers/index.md) to the main app.

## Forward Problem Sub-App

In the forward app, a heat conduction problem is solved with convective flux
type boundary condition that changes with the inner radius. The outer radius is
insulated and there is a constant source term throughout. The objective is
find the two radii that will have the lowest max temperature while satisfying a
volume constraint.

In the forward app, the two optimization parameters control the inner and outer
values are used to displace the boundaries of the
[ParsedOptimizationFunction](/Functions/ParsedOptimizationFunction) to calculate
the cartesian boundary displacement. These
values are used displace the boundaries of the
annulus to the correct position using Dirichlet boundary conditions. A diffusion
problem on the displacements then allow for the interior elements to be
smoothed. For the temperature field, the physics is solved on the displaced mesh
to calculate the current objective.

!listing test/tests/executioners/constrained/shape_optimization/forward.i
         block= Functions BCs Kernels
         id=forward_app_shape
         caption= Forward App Shape Optimization

For the equality constraints, a gradient of the constraint with respect to the
parameters is needed for TAO. The gradient is analytically computed for the
current radii.

Using postprocessors, the objective and constraint functions can be calculated.
The objective in this case is to minimize the maximum temperature. Also the
current volume is calculated and how much that volume violates the constraint.
These values are now able to be transferred to the main application for the
optimization process.


!listing test/tests/executioners/constrained/shape_optimization/forward.i
         block=Postprocessors
         id=forward_app_obj
         caption= Forward App Objective Calculations

## Main-App Optimization

Optimization problems are solved using the [MultiApps](MultiApps/index.md)
system.  The main application contains the optimization executioner and the
sub-application solve the forward problem.

For this optimization example, the [GeneralOptimization.md] reporter is used. The
[!param](/OptimizationReporter/GeneralOptimization/equality_names) option lists
equality constraints. There is also a
[!param](/OptimizationReporter/GeneralOptimization/inequality_names), not shown
below, for listing the inequality constraints. The
[GeneralOptimization.md] also expects
the controllable parameters
[!param](/OptimizationReporter/GeneralOptimization/objective_name) and
[!param](/OptimizationReporter/GeneralOptimization/num_values_name), which
are the names of the reporters that will hold the objective value and the number
of parameters in the forward problem.


!listing test/tests/executioners/constrained/shape_optimization/main.i
         block=OptimizationReporter
         id=OptRep
         caption= Main application `OptimizationReporter` block with equality
         constraints

To enable constrained optimization the
[!param](/Executioner/Optimize/tao_solver) needs to be set to `taoalmm`, which
is an Augmented Lagrangian method. This is the only TAO solver in MOOSE that has
constraint support. The `taoalmm` subsolver is set with [!param](/Executioner/Optimize/petsc_options_iname) and
[!param](/Executioner/Optimize/petsc_options_value).
This example computes the objective function's gradient using finite differencing in TAO with the `petsc_options_iname` `-tao_fd`.  Finite differencing only uses the forward problem's objective value and does not require an adjoint solve.
`-tao_almm_subsolver_tao_type` and  `bqnktr` set the subsolver to the Bounded Quasi-Newton-Krylov
Trust Region Method.

!listing test/tests/executioners/constrained/shape_optimization/main.i
        block= Executioner
        id=exec
        caption= Main application `Executioner` block

## Adjoint Problem Sub-App

Since we are using finite-differencing to determine the gradients of our
objective function, a Adjoint Sub-App is not needed.

## Optimization Results

As shown in [setup], the initial annulus design has a temperature profile that
reaches a maximum temperature of around 470. The initial inner and outer radii
for this design were 6 and 10, respectively.

!media large_media/optimization/constrainedOpt_shapeSetup.png
        style=width:60%;margin:auto;padding-top:2.5%;background-color: white;color: black;
        id=setup
        caption= Initial annulus temperature profile

After optimizing the design to decrease the maximum temperature, the new design
has a maximum temperature of around 160. Moreover, the volume constraint was
satisfied to the given tolerance. The optimized inner and outer radii are now
1.199 and 8.075, respectively.

[opt_sol] shows the optimized designs and the new temperature profile.

!media large_media/optimization/constrainedOpt_shapeSoln.png
        style=width:60%;margin:auto;padding-top:2.5%;background-color: white;color: black;
        id=opt_sol
        caption= Optimized Annulus temperature profile

