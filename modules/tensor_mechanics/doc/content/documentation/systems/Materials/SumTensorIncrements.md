# Sum Tensor Increments

!syntax description /Materials/SumTensorIncrements

## Descriptions

The `SumTensorIncrements` material updates a strain tensor by summing coupled
strain increments as specified by the user.
\begin{equation}
  \label{eqn:sum_increment_tensor}
  \boldsymbol{T} = \boldsymbol{T}_{old} + \sum_n \Delta D_n
\end{equation}
where $\boldsymbol{T}$ is the calcuated tensor and $\boldsymbol{D}_n$ are the coupled tensor
increments.

## Example Input File

!listing modules/combined/test/tests/DiffuseCreep/stress.i block=Materials/diffuse_creep_strain

where the argument for the `coupled_tensor_increment_names` parameter in the
`SumTensorIncrements` material is the same as the property parameter
`property_name` argument as shown

!listing modules/combined/test/tests/DiffuseCreep/stress.i block=Materials/diffuse_strain_increment

!syntax parameters /Materials/SumTensorIncrements

!syntax inputs /Materials/SumTensorIncrements

!syntax children /Materials/SumTensorIncrements

!bibtex bibliography
