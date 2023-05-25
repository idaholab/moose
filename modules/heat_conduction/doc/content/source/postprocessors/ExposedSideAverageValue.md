# ExposedSideAverageValue

!syntax description /Postprocessors/ExposedSideAverageValue

This is used to compute the average of a variable over only the portion of a specified surface that is exposed, using the [SelfShadowSideUserObject](SelfShadowSideUserObject.md) to determine which portions of the surface are exposed and which are shadowed. This computation is identical to that performed in [SideAverageValue](SideAverageValue.md), except that only portions of the surface that are exposed (i.e., not shadowed) are included in the calculation.

!syntax parameters /Postprocessors/ExposedSideAverageValue

!syntax inputs /Postprocessors/ExposedSideAverageValue

!syntax children /Postprocessors/ExposedSideAverageValue
