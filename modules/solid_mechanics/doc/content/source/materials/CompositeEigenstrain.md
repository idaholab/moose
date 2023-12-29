# Composite Eigenstrain

!syntax description /Materials/CompositeEigenstrain

## Description

The material `CompositeEigenstrain` calculates a Rank-2 tensor that can be used as an Eigenstrain
tensor in a mechanics simulation.  This tensor is computed as a weighted sum of base Eigenstrain
tensors, as shown in [eq:weighted_rank_two], where each weight can be a scalar material property that
may depend on simulation variables.
\begin{equation}
  \label{eq:weighted_rank_two}
  \boldsymbol{T}^{composite} = \sum_n w_n \cdot \boldsymbol{T}_n
\end{equation}
where $\boldsymbol{T}$ is a Rank-2 tensor and $w$ is the weighting factor for each Rank-2 tensor.


!alert warning When using the [`CompositeEigenstrain`](CompositeEigenstrain.md)
object for RankTwoTensor eigenstrains with the
[TensorMechanicsAction](TensorMechanics/Master/index.md) setting
`automatic_eigenstrain_names = true`, eigenstrains listed as `MaterialADConverter`
input tensors will not be included in the `eigenstrain_names` list passed. Set
the automatic/_eigenstrain/_names = false and populate this list manually if
these components need to be included.

## Example Input File Syntax

!listing modules/combined/test/tests/eigenstrain/composite.i block=Materials/eigenstrain

The `eigenstrain_name` parameter value must also be set for the strain calculator, and an example
parameter setting is shown below:

!listing modules/combined/test/tests/eigenstrain/composite.i block=Materials/strain

!syntax parameters /Materials/CompositeEigenstrain

!syntax inputs /Materials/CompositeEigenstrain

!syntax children /Materials/CompositeEigenstrain
