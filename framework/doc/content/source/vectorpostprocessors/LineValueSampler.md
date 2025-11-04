# LineValueSampler

!syntax description /VectorPostprocessors/LineValueSampler

## Description

LineValueSampler samples the given variables/auxvariables at equally spaced points between the start and end points of a user provided line segment. The sampled points and the values are written to a csv file at every time step. The sorting order of the points can be changed using the `sort_by` parameter which takes `x`, `y`, `z` or `id` (increasing distance from start point) as input.

LineValueSampler could also be used as an UserObject with the [MultiAppUserObjectTransfer.md] to transfer values to AuxVariables in the master or sub application. When using the LineValueSampler with the [MultiAppUserObjectTransfer.md], an error is generated if more than one variable is supplied as input to the LineValueSampler as the transfer currently works only with one variable. Also, when calculating the value of the UserObject (LineValueSampler in this case) at a given point, the point is first projected onto the user defined line segment and the interpolated value at the projected point is returned as output. If the projected point falls outside the line segment, infinity is returned as output.

If the variable to be plotted needs to be scaled, this can be done by supplying a postprocessor. Caution should be used to make sure that the postprocessor is being evaluated in such a way that its value will not be lagged when being called by LineValueSampler.

!alert note
If the line value sampler is used with a discontinuous variable on the edge/face of a 2D/3D element, then the value from the element with the lowest ID will be returned.

!alert note title=Vector names / CSV output column names
`LineValueSampler` declares a vector for each spatial coordinate, (`x`, `y`, `z`), of the sampled points,
the distance along the sampled line in a vector called `id`,
and a vector named after each value sampled, containing the variable values at each point.

!alert note title=General sampling
The `LineValueSampler` samples variables on the specified line. For more flexible sampling,
use the [PositionsFunctorValueSampler.md].

!syntax parameters /VectorPostprocessors/LineValueSampler

!syntax inputs /VectorPostprocessors/LineValueSampler

!syntax children /VectorPostprocessors/LineValueSampler
