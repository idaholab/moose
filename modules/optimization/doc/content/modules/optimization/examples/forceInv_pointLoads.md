# Force Inversion Example: Point Loads

## Background

The MOOSE optimization module provides a flexible framework for solving inverse optimization problems in MOOSE.  This page is part of a set of examples for different types of inverse optimization problems.

- [Theory](theory/InvOptTheory.md)
- [Examples overview](examples/index.md)
- [Example 1: Point Loads](forceInv_pointLoads.md)
- [Example 2: Neumann Boundary Condition](forceInv_NeumannBC.md)
- [Example 3: Distributed Body Load](forceInv_BodyLoad.md)

# Example 1: Point Loads id=sec:pointLoad

In this example we are parameterizing the heat source intensity at the locations indicated by the $\bigcirc$ symbols in [figSetup] that will produce a temperature field that most closely matches the temperature measurements taken at the points indicated by the $\times$ symbols.  Dirichlet boundary conditions are applied to the entire boundary with T=300.

!media large_media/optimization/fig_setup.png
       style=width:40%;margin:auto;padding-top:2.5%;
       id=figSetup
       caption=Meshed geometry with measurement ($\times$) and parameterized point load ($\bigcirc$) locations.

## Main Application Input

Optimization problems are solved using the [MultiApps](MultiApps/index.md) system.  The main application contains the optimization executioner and the sub-applications solve the forward and adjoint PDE.   The main application input is shown in [fig:master_app].

!listing test/tests/optimizationreporter/point_loads/main.i
         id=fig:master_app
         caption=Main application optimization input for point load parameterization shown in [figSetup]

The main application runs the optimization executioner and transfers data from the optimization executioner back and forth to the sub-apps that are running the "forward" and "adjoint" FE models.  Since no mesh or physics kernels are required on the main-app, we use the `[StochasticTools]` action to set up all of the nullkernels, empty mesh, etc. needed to get a MOOSE simulation to run.

The [optimize](Optimize.md) executioner block, shown below, provides an interface with the [PETSc TAO](https://www.mcs.anl.gov/petsc/documentation/taosolvertable.html) optimization library. The optimization algorithm is selected with `tao_solver` with specific solver options set with `petsc_options`.


!listing test/tests/optimizationreporter/point_loads/main.i
         block=Executioner

The `Optimize` executioner requires a [OptimizationReporter](syntax/OptimizationReporter/index.md), shown below, to transfer data between the optimization executioner and the transfers used for communicating with the sub-apps.  The type of `OptimizationReporter` depends on the optimization algorithm, given by teh

!listing test/tests/optimizationreporter/point_loads/main.i
         block=OptimizationReporter



The optimization executioner on the main app does not need a mesh or variables and uses the action to set-up a dummy mesh and variables.

!media large_media/optimization/fig_forward.png
      style=width:60%;margin:auto;padding-top:2.5%;
      id=figForward
      caption=Optimization results for the (a) temperature field being matched at the $\times$ symbols in [figSetup] and (b) the intensity of the parameterized point loads applied at the $\bigcirc$ needed to produce the temperature field in (a)

!media large_media/optimization/fig_adjoint.png
      style=width:60%;margin:auto;padding-top:2.5%;
      id=figAdjoint
      caption=Adjoint temperature field and misfit load from first optimization iteration.

The first step is to define the simulation to perform, in this case the simulation considered is a 2D
steady state diffusion equation with Dirichlet boundary conditions on each end of the domain: find $u$
such that

!equation
\frac{\partial u}{\partial t} + \frac{\partial^2 u}{\partial x^2} = 0 \quad x \in [0,1],

where $u(0) = U(0, 0.5)$, $u(1) = U(1,2)$, and $U(a,b)$ defines a
[continuous uniform distribution](https://en.wikipedia.org/wiki/Uniform_distribution_%28continuous%29)
with $a$ and $b$ defining the lower and upper limits of the distribution, respectively.



## Sub-Application Input

The problem defined above, with respect to the [MultiApps](MultiApps/index.md) system, is a sub-application. The
complete input file for the problem is provided in "main-app". The only item required
to enable the stochastic analysis is the [Controls] block, which contains a
[SamplerReceiver](/SamplerReceiver.md) object, the use of which will be explained
in the following section.

!listing test/tests/optimizationreporter/point_loads/main.i
         id=master_app
         caption=Complete input file for executing the transient diffusion problem.

!listing test/tests/optimizationreporter/point_loads/forward.i
        id=forward_app
        caption=Complete input file for executing the transient diffusion problem.

!listing test/tests/optimizationreporter/point_loads/adjoint.i
        id=adjoint_app
        caption=Complete input file for executing the transient diffusion problem.

## Master Input

The master application, with respect to the [MultiApps](MultiApps/index.md) system, is the driver of the stochastic
simulations, by itself it does not perform a solve. The complete input file for the master
application is shown in [master_app], but the import sections will be detailed individually.

First, [OptimizationReporter](syntax/OptimizationReporter/index.md) are defined.

!listing test/tests/optimizationreporter/point_loads/main.i block=OptimizationReporter

