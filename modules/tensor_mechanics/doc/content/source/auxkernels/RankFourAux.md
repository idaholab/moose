# Rank Four Aux

!syntax description /AuxKernels/RankFourAux

## Description

The AuxKernel `RankFourAux` is used to save single components of Rank-4 tensors into an AuxVariable
for visualization and/or post-processing purposes, similar to the functionality provided by
[RankTwoAux](/RankTwoAux.md) for Rank-2 tensors `RankFourAux` is commonly used to output components
of the elasticity (or stiffness) tensor, $C_{ijkl}$, in mechanics simulations.

The `RankFourAux` takes as arguments the values of the `index_i`, `index_j`, `index_k`, and `index_l` for the single Rank-4 tensor component to save into an AuxVariable.
\begin{equation}
\label{eq:rank4tensor_aux_indices}
  \begin{aligned}
        C_{ijkl} \implies & \underbrace{\begin{bmatrix}
                      C_{11} & C_{12} & C_{13} & C_{14} & C_{15} & C_{16} \\
                      C_{21} & C_{22} & C_{23} & C_{24} & C_{25} & C_{26} \\
                      C_{31} & C_{32} & C_{33} & C_{34} & C_{35} & C_{36} \\
                      C_{41} & C_{42} & C_{43} & C_{44} & C_{45} & C_{46} \\
                      C_{51} & C_{52} & C_{53} & C_{54} & C_{55} & C_{56} \\
                      C_{61} & C_{62} & C_{63} & C_{64} & C_{65} & C_{66}
                      \end{bmatrix}}_{\text{textbook notation}} \\[5.0em]
         \implies & \underbrace{\begin{bmatrix}
                      C_{0000} & C_{0011} & C_{0022} & C_{0012} & C_{0020} & C_{0001} \\
                      C_{1100} & C_{1111} & C_{1122} & C_{1112} & C_{1120} & C_{1101} \\
                      C_{2200} & C_{2211} & C_{2222} & C_{2212} & C_{2220} & C_{2201} \\
                      C_{1200} & C_{1211} & C_{1222} & C_{1212} & C_{1220} & C_{1201} \\
                      C_{2000} & C_{2011} & C_{2022} & C_{2012} & C_{2020} & C_{2001} \\
                      C_{0100} & C_{0111} & C_{0122} & C_{0112} & C_{0120} & C_{0101}
                      \end{bmatrix}}_{\text{Rank Four Aux indices}}
  \end{aligned}
\end{equation}
[eq:rank4tensor_aux_indices] shows the index values for a linear hyperelastic stiffness tensor with
21 indepent material parameters; the various available elasticity tensor symmetry options is
discussed in the material [ComputeElasticityTensor](/ComputeElasticityTensor.md)
documentation.

## Example Input File Syntax

!listing modules/combined/test/tests/linear_elasticity/tensor.i block=AuxKernels/matl_C25

An AuxVariable is required to store the `RankFourAux` AuxKernel information. Note that the name of
the AuxVariable is used as the argument for the `variable` input parameter in the `RankFourAux`
block.

!listing modules/combined/test/tests/linear_elasticity/tensor.i block=AuxVariables/C25

!syntax parameters /AuxKernels/RankFourAux

!syntax inputs /AuxKernels/RankFourAux

!syntax children /AuxKernels/RankFourAux
