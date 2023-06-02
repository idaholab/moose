# Debug System

## Overview

The `[Debug]` input file block is designed to contain options to enable debugging tools for a
simulation. For example, the input file snippet below demonstrates how to enable the material
property debugging tool. This tool automatically outputs material properties as fields in the
[outputs/Exodus.md] file.
A complete list of the available options is provided in [#debug-params].

!listing show_material_props_debug.i block=Debug

## Residual outputs for debugging nonlinear convergence issues

When solving multi-variable or multi-physics problems, it is often the case that the residual for a
subset of variables is more problematic to converge than for the others. This may be because the underlying
physics are tougher to solve, or because there are issues with the kernels for those variables!

MOOSE provides two convenient debug boolean options to examine the convergence of nonlinear residuals:

- [!param](/Debug/show_var_residual_norms) shows the residual norms for each nonlinear variable
  equation. The equation with the highest residual is the least converged.
  This is the norm after scaling if equation scaling, automatic or manual, is used.

- [!param](/Debug/show_top_residuals) shows the residual norms only for the least converged variable equation.


Helpful information on debugging numerical convergence issues is provided in the [numerical troubleshooting page](application_usage/failed_solves.md optional=True).

## Execution ordering output

### Ordering of the problem set-up id=debug-setup

MOOSE parses the input file and executes numerous [Actions](actions/Action.md) which progressively
load/build the mesh, create the variables, kernels, boundary conditions, output objects etc.
The ordering of this process may be shown using the [!param](/Debug/show_actions) parameter.

!alert note
The dependencies of each `Action` should be declared in the source code of each `Action`. This enables MOOSE
to perform automatic dependency resolution to correctly order them.

!alert note
For the automatic ordering of the mesh generators, please refer to the
[mesh documentation page](syntax/Mesh/index.md).

### Solve and execution ordering id=debug-order

Nearly every solve in MOOSE consists of a succession of operations on nodes, quadrature points,
elements and elements' faces. These operations may be for example to compute the contribution of a
kernel/boundary condition/etc. to the residual, Jacobian, etc.

The MOOSE `Debug` system offers the [!param](/Debug/show_execution_order) parameter to output the
order of execution of each of these objects in those loops. This order may depend on local block/boundary
restrictions, and local or global dependency resolutions.

This parameter is most helpful to understand if `AuxKernels`, `UserObjects` and other systems which can
interact in arbitrarily complex ways on a group of variables are indeed executed in the order desired
by the modeler. If problematic, object execution may be reordered using various boolean parameters, `execute_on` flags, and other manual dependency declarations.
For example for UserObjects, the `execution_order_group` parameter lets the modeler select the ordering of executions of user objects.

## Other useful outputs available in other systems

The `[Debug]` system is not the only system that provides useful debugging information. We summarize
these other helpful resources below:

- to debug [MultiApps](syntax/MultiApps/index.md) and [Transfers](syntax/Transfers/index.md)-related
  issues, the `FEProblem` parameter [!param](/Problem/FEProblem/verbose_multiapps) shows a helpful summary of
  transfers executed and important metadata about each `Transfer`.

- to debug linear system convergence issues, numerous parameters may be passed to PETSc to make it more verbose.
  They are summarized on this [page about debugging numerical issues](application_usage/failed_solves.md optional=True) and in
  the [PETSc manual](https://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/).

- to debug a [Mesh](syntax/Mesh/index.md) related issue, please see [this page](syntax/Mesh/index.md#troubleshooting)
  for built-in MOOSE mesh generation issues and [this page](syntax/Mesh/index.md#issues) for possible issues from an external mesh.


!alert note
There are currently no convenient debugging options or tools for `MultiApps`-based fixed point iteration problems.
Use the [!param](/Executioner/Transient/fixed_point_min_its), [!param](/Executioner/Transient/fixed_point_max_its) and
[!param](/Executioner/Transient/accept_on_max_fixed_point_iteration) to output at the desired fixed point iteration.


## Parameters list

!syntax parameters /Debug id=debug-params

## Syntax list

!syntax list /Debug objects=True actions=False subsystems=False

!syntax list /Debug objects=False actions=False subsystems=True

!syntax list /Debug objects=False actions=True subsystems=False
