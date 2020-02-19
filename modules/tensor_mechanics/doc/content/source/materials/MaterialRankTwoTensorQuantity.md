#Material Rank Two Tensor Quantity

!syntax description /Materials/MaterialRankTwoTensorQuantity

## Description

Material Rank Two Tensor Quantity is used within
[TensorMechanics/Master](/Modules/TensorMechanics/Master/index.md) for problems
with a Cartesian coordinate system. This class provides methods to calculate
several different scalar stress ($\boldsymbol{\sigma}$) and strain
($\boldsymbol{\epsilon}$)quantities for a Rank-2 tensor, as described in
[RankTwoScalarTools](RankTwoScalarTools.md).  

The `MaterialRankTwoTensorQuantity` takes as arguments the values of the
`index_i` and the `index_j` for the single tensor component to save into an
MaterialProperty.  [eq:mat_rank2tensor_quantity_indices] shows the index values
for each Rank-2 tensor component.
\begin{equation}
\label{eq:mat_rank2tensor_quantity_indices}
\sigma_{ij} \implies \begin{bmatrix}
                      \sigma_{00} & \sigma_{01} & \sigma_{02} \\
                      \sigma_{10} & \sigma_{11} & \sigma_{12} \\
                      \sigma_{20} & \sigma_{21} & \sigma_{22}
                      \end{bmatrix}
\end{equation}

If desired, `MaterialRankTwoTensorQuantity` can be restricted to save data from
a Rank-2 tensor at a single specified quadrature point per element. This option
is generally used for debugging purposes.

!syntax parameters /Materials/MaterialRankTwoTensorQuantity

!syntax inputs /Materials/MaterialRankTwoTensorQuantity

!syntax children /Materials/MaterialRankTwoTensorQuantity
