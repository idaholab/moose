# CZMRealVectorScalar

!syntax description /Materials/CZMRealVectorScalar

## Description

This is a Material model used to extract the normal or tangent norm of a real vector material defined on a cohesive zone. This can be used only for vector defined on the global coordinate system, like the `traction_global` or the `displacement_jump_global` vectors.

!alert warning
Behind the scene, this material uses the `czm_total_rotation` matrix to compute the tangent or normal scalar values, so it should not be used for vector quantities defined in interface coordinates.

This Material model is set up by [CohesiveZoneMaster](CohesiveZoneMaster/index.md) automatically
when the normal or tangent traction/displacement_jump scalar are requested in the generate_output parameter, but can also be set up directly by the user.  

The `CZMRealVectorScalar` takes as arguments the `real_vector_value`  for which the scalar component needs to be computed, the `direction`, and the `property_name` assigned to the computed scalar.
Allowed values for `direction` are: `Normal`, and `Tangent`.
The normal signed magnitude is defined as:
\begin{equation}
V_n = (V \cdot n)
\end{equation}
where $n$ is the interface normal, and $V$ is the input real vector.
The tangent norm is defined as
\begin{equation}
V_t = \lvert V - V_n \cdot n \rvert_2
\end{equation}

!syntax parameters /Materials/CZMRealVectorScalar

!syntax inputs /Materials/CZMRealVectorScalar

!syntax children /Materials/CZMRealVectorScalar
