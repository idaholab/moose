# WaveSpeedVPP

!syntax description /VectorPostprocessors/WaveSpeedVPP

## Overview

This object takes `elem_id` and `side_id` input parameters and outputs the wave
speeds associated with them. The user has to be careful when using this
object. They should be aware of the following:

- A `FaceInfo` object may not be associated with the given `elem_id` and
  `side_id`. For instance if the `FaceInfo` lives on a face associated with
  `elem_id = 0` and `side_id = 1` on the left and `elem_id = 1` and `side_id = 0`
  on the right, then `FaceInfo` data will only be retrievable for the `elem_id =
  0` and `side_id = 1` pair because `FaceInfo` objects are always associated with
  lower element id. So if in this case a user requested wave speeds for
  `elem_id = 1` and `side_id = 0` no wave speeds would be output.
- Element numbering will be different between replicated and distributed mesh
  modes so even if a user gets the desired output for a given `elem_id` and
  `side_id` pair when running with a replicated mesh, they may not get any output
  when running with a distributed mesh for the reason stated in the above bullet

!syntax parameters /VectorPostprocessors/WaveSpeedVPP

!syntax inputs /VectorPostprocessors/WaveSpeedVPP

!syntax children /VectorPostprocessors/WaveSpeedVPP
