# Rank Two Scalar Aux

!syntax description /AuxKernels/RankTwoScalarAux

## Description

This AuxKernal uses a set of functions to compute scalar quantities such as
invariants and components in specified directions from rank-2 tensors such as
stress or strain.  See [RankTwoScalarTools](RankTwoScalarTools.md) for further
information.

### Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/material_limit_time_step/elas_plas/nafems_nl1_lim.i
         block=AuxKernels/vonmises

An AuxVariable is required to store the AuxKernel information. Note that the name of the AuxVariable
is used as the argument for the `variable` input parameter in the `RankTwoScalarAux` block.

!listing modules/tensor_mechanics/test/tests/material_limit_time_step/elas_plas/nafems_nl1_lim.i
         block=AuxVariables/vonmises

As with the [RankTwoAux](/RankTwoAux.md) AuxKernel, `RankTwoScalarAux` requires the inclusion of an
AuxVariable block for each AuxKernel block.

## AuxVariable Order

!alert! note title=Elemental vs Nodal Visualization of Quadrature Field Values
Results will have different quality based on the AuxVariable:

- +Elemental Constant Monomial+ Using an AuxVariable with `family = MONOMIAL` and `order = CONSTANT` will give a constant value of
  the AuxVariable for the entire element, which is computed by taking a volume-weighted average of the integration
  point quantities. This is the default option using TensorMechanics Action and requires the least computational cost.
- +Elemental Higher-order Monomial+ Using an AuxVariable with `family = MONOMIAL` and `order = FIRST` or higher will result in
  fields that vary linearly (or with higher order) within each element. Because the Exodus mesh format does not
  support higher-order elemental variables, these AuxVariables are output by libMesh as nodal variables for visualization
  purposes. Using higher order monomial variables in this way can produce smoother visualizations of results for a properly
  converged simulation.
- +Nodal Lagrange+ Using an AuxVariable with `family = LAGRANGE` will result in a smooth nodal field of the material property,
  constructed using [nodal patch recovery](nodal_patch_recovery.md optional=True).
  `patch_polynomial_order` is set to equal the order of the AuxVariable by default.
  Use this option for the best (smoothest, most accurate) results, but there is
  some additional computational cost. Furthermore, this method is suitable +only
  for serial simulations+ at present.
!alert-end!

!syntax parameters /AuxKernels/RankTwoScalarAux

!syntax inputs /AuxKernels/RankTwoScalarAux

!syntax children /AuxKernels/RankTwoScalarAux

!bibtex bibliography
