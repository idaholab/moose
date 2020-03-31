# PhaseNormalTensor

!syntax description /Materials/PhaseNormalTensor

Computes the second order tensor $P$ (with a property name given by `normal_tensor_name`)

!equation
P = \frac{\nabla v \otimes \nabla v}{|\nabla v|^2}

where $\otimes$ is the outer product of two vectors. The resulting tensor has a
major axis that is parallel to the gradient of the coupled variable $v$
(`phase`), a norm of 1, and minor axis of zero.

!syntax parameters /Materials/PhaseNormalTensor

!syntax inputs /Materials/PhaseNormalTensor

!syntax children /Materials/PhaseNormalTensor
