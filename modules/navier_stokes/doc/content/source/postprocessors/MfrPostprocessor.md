# MfrPostprocessor

!syntax description /Postprocessors/MfrPostprocessor

## Overview

This object is a receptor for boundary mass flow rates computed in advection
boundary condition objects derived from `FVFluxBC`. As long as the
`mfr_postprocessor` parameter is set in the BC object, this
object will do the right thing, e.g. the boundary condition object will set the
mass flow rate on a per-`FaceInfo` basis and then this object will perform the
aggregation process over processes and threads.

!syntax parameters /Postprocessors/MfrPostprocessor

!syntax inputs /Postprocessors/MfrPostprocessor

!syntax children /Postprocessors/MfrPostprocessor
