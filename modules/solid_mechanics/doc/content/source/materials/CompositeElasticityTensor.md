# Composite Elasticity Tensor

!syntax description /Materials/CompositeElasticityTensor

## Description

`CompositeElasticityTensor` calculates a simple Rank-4 tensor that can be used as an Elasticity
tensor in a mechanics simulation.  This tensor is computed as a weighted sum of base elasticity
tensors, as shown in [eq:weighted_rank_four], where each weight can be a scalar material
property that may depend on simulation variables.
\begin{equation}
  \label{eq:weighted_rank_four}
  \boldsymbol{T}^{composite} = \sum_n w_n \cdot \boldsymbol{T}_n
\end{equation}
where $\boldsymbol{T}$ is a Rank-4 tensor and $w$ is the weighting factor for each Rank-4 tensor.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/elasticitytensor/composite.i block=Materials/C

!syntax parameters /Materials/CompositeElasticityTensor

!syntax inputs /Materials/CompositeElasticityTensor

!syntax children /Materials/CompositeElasticityTensor
