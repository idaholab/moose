# PointValue

The PointValue Postprocessor samples a field variable at a single point within the domain. It will throw
an error if the point being sampled lies outside of the domain.

!alert warning
The behavior of the PointValue processor is undefined if using discontinous shape functions
and the sample point lies right on a discontinuity.

!syntax parameters /Postprocessors/PointValue

!syntax inputs /Postprocessors/PointValue

!syntax children /Postprocessors/PointValue
