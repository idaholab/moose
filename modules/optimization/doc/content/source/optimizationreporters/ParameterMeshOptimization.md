# ParameterMeshOptimization

!syntax description /OptimizationReporter/ParameterMeshOptimization

## Overview

This optimization reporter performs the same type of optimization as [OptimizationReporter.md], except the parameters are defined by a mesh and is meant to be used in conjunction with [ParameterMeshFunction.md]. The idea is that the parameters are defined as variables on the inputted mesh and the resulting values on the problem being optimized is based on the finite-element shape functions of the variable. The parameter mesh does not need to conform to the physics mesh, but every point in the physics mesh must be contained in parameter mesh. The parameters are defined with [!param](/OptimizationReporter/ParameterMeshOptimization/parameter_names) and each one needs to have an associated

- mesh ([!param](/OptimizationReporter/ParameterMeshOptimization/parameter_meshes)),
- finite-element family ([!param](/OptimizationReporter/ParameterMeshOptimization/parameter_families)) where a single value input is applied to all parameters,
- finite-element order ([!param](/OptimizationReporter/ParameterMeshOptimization/parameter_orders)) where a single value input is applied to all parameters,
- initial condition ([!param](/OptimizationReporter/ParameterMeshOptimization/initial_condition)) where the default is 0,
- lower bound ([!param](/OptimizationReporter/ParameterMeshOptimization/lower_bounds)) where the default is no bound, and
- upper bound ([!param](/OptimizationReporter/ParameterMeshOptimization/upper_bounds)) where the default is no bound.

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

!syntax parameters /OptimizationReporter/ParameterMeshOptimization

!syntax inputs /OptimizationReporter/ParameterMeshOptimization

!syntax children /OptimizationReporter/ParameterMeshOptimization
