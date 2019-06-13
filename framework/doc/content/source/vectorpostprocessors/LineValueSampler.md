# LineValueSampler

!syntax description /VectorPostprocessors/LineValueSampler

## Description

LineValueSampler samples the given variables/auxvariables at equally spaced points between the start and end points of a user provided line segment. The sampled points and the values are written to a csv file at every time step. The sorting order of the points can be changed using the `sort_by` parameter which takes `x`, `y`, `z` or `id` (increasing distance from start point) as input.

LineValueSampler could also be used as an UserObject with the [MultiAppUserObjectTransfer](/MultiAppUserObjectTransfer.md) to transfer values to AuxVariables in the master or sub application. When using the LineValueSampler with the MultiAppUserObjectTransfer, an error is generated if more than one variable is supplied as input to the LineValueSampler as the transfer currently works only with one variable. Also, when calculating the value of the UserObject (LineValueSampler in this case) at a given point, the point is first projected onto the user defined line segment and the interpolated value at the projected point is returned as output. If the projected point falls outside the line segment, infinity is returned as output.

If the variable to be plotted needs to be scaled, this can be done by supplying a postprocessor. Caution should be used to make sure that the postprocessor is being evaluated in such a way that its value will not be lagged when being called by LineValueSampler.

!syntax parameters /VectorPostprocessors/LineValueSampler

!syntax inputs /VectorPostprocessors/LineValueSampler

!syntax children /VectorPostprocessors/LineValueSampler

!bibtex bibliography
