# Rank Two Aux

!syntax description /AuxKernels/RankTwoAux

## Description

The AuxKernel `RankTwoAux` is used to save single components of Rank-2 tensors into an AuxVariable
for visualization and/or post-processing purposes. An antisymmetric Rank-2 tensor would require nine
separate `RankTwoAux` AuxKernel-AuxVariable pairs to store all of the components of the antisymmetric
Rank-2 tensor; six separate AuxKernel-AuxVariable pairs are required to print out all the unique
components of a symmetric Rank-2 tensor.  Quantities commonly examined with `RankTwoAux` are stress
($\boldsymbol{\sigma}$) and strain ($\boldsymbol{\epsilon}$).

The `RankTwoAux` takes as arguments the values of the `index_i` and the `index_j` for the single
tensor component to save into an AuxVariable.  [eq:rank2tensor_aux_indices] shows the index
values for each Rank-2 tensor component.
\begin{equation}
\label{eq:rank2tensor_aux_indices}
\sigma_{ij} \implies \begin{bmatrix}
                      \sigma_{00} & \sigma_{01} & \sigma_{02} \\
                      \sigma_{10} & \sigma_{11} & \sigma_{12} \\
                      \sigma_{20} & \sigma_{21} & \sigma_{22}
                      \end{bmatrix}
\end{equation}

If desired, `RankTwoAux` can be restricted to save data from a Rank-2 tensor at a single specified
quadrature point per element. This option is generally used for debugging purposes.

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

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/elastic_patch/elastic_patch.i block=AuxKernels/stress_xy

An AuxVariable is required to store the `RankTwoAux` AuxKernel information. Note that the name of the
AuxVariable is used as the argument for the `variable` input parameter in the `RankTwoAux` block.

!listing modules/tensor_mechanics/test/tests/elastic_patch/elastic_patch.i block=AuxVariables/stress_xy

!syntax parameters /AuxKernels/RankTwoAux

!syntax inputs /AuxKernels/RankTwoAux

!syntax children /AuxKernels/RankTwoAux
