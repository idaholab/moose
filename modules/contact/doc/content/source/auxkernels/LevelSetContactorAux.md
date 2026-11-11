# LevelSetContactorAux

!syntax description /AuxKernels/LevelSetContactorAux

## Description

Writes a scalar component of a [LevelSetContactor.md] --- signed distance
or an individual normal component --- into an aux variable at the current
node or quadrature point.  Useful for visualization (contours of the
signed-distance field on the deformable-body surface, for example) and
for downstream postprocessing that needs the level-set field as an
auxiliary variable rather than through the contactor UO's programmatic
interface.

## Parameters

!syntax parameters /AuxKernels/LevelSetContactorAux
