# Constrained Optimization Example: Shape Optimization

## Background

The MOOSE Optimization module provides a flexible framework for solving
optimization problems in MOOSE. This page will demonstrate constrained shape
optimization of an annulus while satisfying an equality volume constraint.

# Example: Shape Optimization

## Main-App Optimization

Optimization problems are solved using the [MultiApps](MultiApps/index.md)
system.  The main application contains the optimization executioner and the
sub-applications solve the forward and adjoint PDE.   The main application input
is shown in [main_app].

!listing examples/shape_optimization/main.i
         id=main_app
         caption= Constrained Optimization Main App

For this optimization example, the [GeneralOptimization](/OptimizationReporter/GeneralOptimization) reporter is used. The
[!param](/OptimizationReporter/OptimizationReporter/equality_names) option lists
equality constraints. There is also a
[!param](/OptimizationReporter/OptimizationReporter/inequality_names), not shown
below, for listing the inequality constraints. The
[GeneralOptimization](/OptimizationReporter/GeneralOptimization) also expects
the parameters
[!param](/OptimizationReporter/GeneralOptimization/objective_name) and
[!param](/OptimizationReporter/GeneralOptimization/number_dofs_reporter), which
are the names of the reporters that will hold the objective value and the number
of parameters in the forward problem.


!listing examples/shape_optimization/main.i
         block=OptimizationReporter
         id=OptRep
         caption= Main application `OptimizationReporter` block with equality
         constraints

To enable constrained optimization the
[!param](/Executioner/Optimize/tao_solver) needs to be set to `taoalmm`. This
the only TAO solver that MOOSE currently interfaces to that has general
constraint support. Using the
[!param](/Executioner/Optimize/petsc_options_iname) and
[!param](/Executioner/Optimize/petsc_options_value) the type of subsolver that
the `taoalmm` solver will use. In this example, the subsolver was set by
`-tao_almm_subsolver_tao_type` and  `bqnktr` to the Bounded Quasi-Newton-Krylov
Trust Region Method. Later on in the forward app we show that the objective
function's gradient is not calculated explicitly. So a finite differencing scheme
is used for the gradient calculation.

!listing examples/shape_optimization/main.i
        block= Executioner
        id=exec
        caption= Main application `Executioner` block

## Forward Problem Sub-App

In the forward app, a heat conduction problem is solved with convective flux
type boundary condition that changes with the inner radius. The objective is
find the two radii that will have the lowest max temperature while satisfying a
volume constraint.

!listing examples/shape_optimization/forward.i
         id=forward_app
         caption= Constrained Optimization Forward SubApp

In the forward app, the two optimization parameters control the inner and outer
radius of the annulus. These parameters are used by the
[ParsedOptimizationFunction](/Functions/ParsedOptimizationFunction) to calculate
the cartesian boundary displacement. These
values are used displace the boundaries of the
annulus to the correct position using Dirichlet boundary conditions. A diffusion
problem on the displacements then allow for the interior elements to be
smoothed. For the temperature field, the physics is solved on the displaced mesh
to calculate the current objective.

For the equality constraints, a gradient of the constraint with respect to the
parameters is needed for TAO. The gradient is analytically computed for the
current radii.

!listing examples/shape_optimization/forward.i
         block= Functions BCs Kernels
         id=forward_app_shape
         caption= Forward App Shape Optimization

Using postprocesors, the objective and constraint functions can be calculated.
The objective in this case is to minimize the maximum temperature. Also the
current volume is calculated and how much that volume violates the constraint.
These values are now able to be transferred to the main application for the
optimization process.


!listing examples/shape_optimization/forward.i
         block=Postprocessors
         id=forward_app_obj
         caption= Forward App Objective Calculations

## Adjoint Problem Sub-App

Since we are using finite-differencing to determine the gradients of our
objective function, a Adjoint Sub-App is not needed.

## Optimization Results

As shown in [setup], the initial annulus design has a temperature profile that
has max temperature around 470.

!media large_media/optimization/constrainedOpt_shapeSetup.png
        style=width:60%;margin:auto;padding-top:2.5%;background-color: white;color: black;
        id=setup
        caption= Initial annulus temperature profile

After optimizing the design to decrease the max
temperature, the new design has a max temperature around 160 and the volume
constraint was satisfied to the tolerance. [opt_sol] shows the optimized designs
and the new temperature profile.

!media large_media/optimization/constrainedOpt_shapeSoln.png
        style=width:60%;margin:auto;padding-top:2.5%;background-color: white;color: black;
        id=opt_sol
        caption= Optimized Annulus temperature profile

