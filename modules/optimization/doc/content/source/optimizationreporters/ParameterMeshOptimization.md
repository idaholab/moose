# ParameterMeshOptimization

!syntax description /OptimizationReporter/ParameterMeshOptimization

## Overview

This optimization reporter performs the same type of optimization as [OptimizationReporter.md], except the parameters are defined by a mesh and is meant to be used in conjunction with [ParameterMeshFunction.md]. The idea is that the parameters are defined as variables on the inputted mesh and the resulting values on the problem being optimized is based on the finite-element shape functions of the variable. The parameter mesh does not need to conform to the physics mesh, but every point in the physics mesh must be contained in parameter mesh. The parameters are defined with [!param](/OptimizationReporter/ParameterMeshOptimization/parameter_names) and each name has the following options assosciated with it:

- mesh ([!param](/OptimizationReporter/ParameterMeshOptimization/parameter_meshes)) (+Required+),
- finite-element family ([!param](/OptimizationReporter/ParameterMeshOptimization/parameter_families)) where a single value input is applied to all parameters,
- finite-element order ([!param](/OptimizationReporter/ParameterMeshOptimization/parameter_orders)) where a single value input is applied to all parameters,
- initial condition which can be set by the following inputs:

  - from the input file using ([!param](/OptimizationReporter/ParameterMeshOptimization/constant_group_initial_condition)) where one value is given for each group of parameters
  - from the parameter mesh using ([!param](/OptimizationReporter/ParameterMeshOptimization/initial_condition_mesh_variable)) where each parameter in the group is initialized from data read from the parameter mesh exodus file,
  - default is zero
  
- lower bound which can be set by the following inputs:

  - from the input file using ([!param](/OptimizationReporter/ParameterMeshOptimization/constant_group_lower_bounds)) where one value is given for each group of parameters
  - from the parameter mesh using ([!param](/OptimizationReporter/ParameterMeshOptimization/lower_bound_mesh_variable)) where each parameter in the group is initialized from data read from the parameter mesh exodus file
  - default is no bounds

- upper bound can be set by the following inputs:

  - from the input file using ([!param](/OptimizationReporter/ParameterMeshOptimization/constant_group_upper_bounds)) where one value is given for each group of parameters
  - from the parameter mesh using ([!param](/OptimizationReporter/ParameterMeshOptimization/upper_bound_mesh_variable)) where each parameter in the group is initialized from data read from the parameter mesh exodus file
  - default is no bounds

[!ref](ParameterMeshFunction.md#tab:fe_types) shows common interpolation types for the parameters.

!alert warning
The mesh created +must+ be replicated. Ensure this by having `Mesh/parallel_type=REPLICATED` when creating the mesh.

## Example Input File Syntax

The first step in doing mesh-based inverse optimization is creating the parameter mesh. The easiest way of doing this is defining the mesh in a separate input file and run it with `--mesh-only` on command-line. The input below creates a two-by-two mesh outputted to exodus as `parameter_mesh_in.e`:

!listing mesh_source/parameter_mesh.i

In the main optimization input, this mesh is then inputted into the ParameterMeshOptimization object:

!listing mesh_source/main.i block=OptimizationReporter

The mesh is also used in the [ParameterMeshFunction.md] objects in the forward and adjoint inputs:

!listing mesh_source/forward.i block=Functions

!listing mesh_source/adjoint.i block=Functions

The transfer of parameters is the same as in other inverse optimization problems, see [OptimizationReporter.md].

## Example Input File Syntax For Restarting An Optimization Simulation

A good guess for the initial parameter values will help with the convergence of the optimization problem.  In this example, initial conditions for the controllable parameters are given on an exodus mesh.  The parameter values on the exodus mesh are the optimized parameter values found in the previous example.  The parameter values from the above optimization problem are output by its forward problem onto the 10x10 mesh used by the FEM simulation.  The forward problem parameter field is then projected onto a coarser two-by-two parameter mesh as a first order Lagrange nodal AuxVariable called `restart_source` using [SolutionUserObject.md] and [SolutionAux.md] in the following input file

!listing mesh_source/parameter_mesh_restart.i

After the parameter mesh is initialized with parameter values, it can be used as an initial guess in the optimization problem.  The AuxVariable `restart_source` is then read in by `OptimizationReporter` using `initial_condition_mesh_variable` as the initial guess for the parameter field:  

!listing mesh_source/main_linearRestart.i block=OptimizationReporter

This optimization simulation is restarted with the optimized parameter values from the first example and convergence occurs in a single step.  The `ParameterMeshOptimization` reporter also defined an `exodus_timesteps_for_parameter_mesh_variable` to identify which step to read from the exodus file.  Constant lower and upper bounds were also given by `constant_group_lower_bounds` and `constant_group_upper_bounds` but do not have an effect on the optimization algorithm because it was started from the exact solution for the parameter field.  To use bounds, the `tao_solver` in the `Executioner` block was changed to `taoblmvm` (bounded lmvm).  For difficult optimization problems, it is better to start with a good guess for the parameters than to tighten up their bounds.  Multi-resolution optimization strategies [!citet](eslaminia2022full) find better initial guesses for the parameters by starting with an easier to solve optimization problem containing a few controllable parameters and then reusing those optimized parameters as an initial guess for a more difficult optimization problem with more parameters.
!syntax parameters /OptimizationReporter/ParameterMeshOptimization

!syntax inputs /OptimizationReporter/ParameterMeshOptimization

!syntax children /OptimizationReporter/ParameterMeshOptimization
