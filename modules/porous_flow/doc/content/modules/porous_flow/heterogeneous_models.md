# Heterogeneous models

Realistic models of porous media often feature spatially varying material properties, especially
porosity and permeability. There are several ways to introduce heterogeneity into the model in
PorousFlow. In the following, we demonstrate how to create models with heterogeneous rock properties
and use these properties in a simulation.

## Regular grids

For regular structured grids that can easily be constructed using common meshing tools (or even the
meshing capability built in to MOOSE), heterogeneity can simply be read from an external data file. In
this example, we consider the 2D model of permeability heterogeneity presented as Case 1 of the tenth
[SPE comparative problem](https://www.spe.org/web/csp/datasets/set01.htm).

The permeability data for this model is read from an ASCII file containing coordinates and permeability
values (in millidarcys):

!listing modules/porous_flow/examples/reservoir_model/spe10_case1.data

A [PiecewiseMultilinear](PiecewiseMultilinear.md) function is used to interpolate the permeability to the
mesh.

!listing modules/porous_flow/examples/reservoir_model/regular_grid.i block=Functions

Constant Monomial AuxVariables are used to store the permeability read from the data file:

!listing modules/porous_flow/examples/reservoir_model/regular_grid.i block=AuxVariables

A [FunctionAux](FunctionAux.md) AuxKernel is used to populate the AuxVariables.

In this example, the permeability in the data file is in millidarcys. As PorousFlow expects permeability
in SI units of m$^2$, we multiply each permeability value by $9.869233 \times 10^{-16}$ and save these
values in a new AuxVariable:

!listing modules/porous_flow/examples/reservoir_model/regular_grid.i block=AuxKernels

As this interpolation and multiplication is only required at the beginning of the simulation, we set
the `execute_on` parameter to `initial` only.

Finally, the heterogeneous permeability (in m$^2$) can be used in the calculation using a [PorousFlowPermeabilityConstFromVar](PorousFlowPermeabilityConstFromVar.md) material.

!listing modules/porous_flow/examples/reservoir_model/regular_grid.i block=Materials/permeability

The above steps create the following heterogeneous model that can then be used in a simulation:

!media media/porous_flow/spe10_case1.png
       id=fig:regular_grid
       style=width:80%;margin-left:10px;
       caption=Heterogeneous permeability for SPE comparative problem case 1

## Reservoir models

Often geological models are created using a modelling package to create realistic interpretations
of the geology. In this case, some pre-processing of the geological model is often required before
it can be used in PorousFlow.

This pre-processing must be performed using some software that can take a geological model and convert
it into a form that PorousFlow can read. One example of a pre-processing code that may be used to achieve
this is [em2ex](https://github.com/cpgr/em2ex). This code converts a reservoir model in either Eclipse format
or cell centered csv format to an Exodus mesh that can used in PorousFlow.

The following example shows how to use an Exodus mesh that includes heterogeneity in a PorousFlow
simulation. For this example, we use publicly available data from the [SAIGUP](https://www.nr.no/saigup)
project to construct the heterogeneous reservoir model shown in [fig:field_model].

!media media/porous_flow/saigup.png
      id=fig:field_model
      style=width:80%;margin-left:10px;
      caption=Heterogeneous permeability for SAIGUP model

The Eclipse reservoir model of the above structure is converted to an Exodus file using
[em2ex](https://github.com/cpgr/em2ex). The heterogeneous porosity and permeability can
then be read from the grid and used in the calculations using the following steps:

First, the mesh (containing the heterogeneous reservoir properties) is read.

!listing modules/porous_flow/examples/reservoir_model/field_model.i block=Mesh

Constant monomial AuxVariables are then created. As the reservoir model again contains permeability
in millidarcys, additional AuxVariables are also declared to hold the permeability in SI units (m$^2$).

The AuxVariables for permeability in millidarcys are given initial values contained
in the mesh file using the `initial_from_file_var` parameter, effectively transferring
the heterogeneity from the mesh file to the AuxVariables.

!listing modules/porous_flow/examples/reservoir_model/field_model.i block=AuxVariables

Like the previous example, the permeability can be converted to SI units using a [ParsedAux](ParsedAux.md)
AuxKernel for each component.

!listing modules/porous_flow/examples/reservoir_model/field_model.i block=AuxKernels

The heterogeneous porosity and permeabilities can then be used in the calculations:

!listing modules/porous_flow/examples/reservoir_model/field_model.i block=Materials/porosity

!listing modules/porous_flow/examples/reservoir_model/field_model.i block=Materials/permeability
