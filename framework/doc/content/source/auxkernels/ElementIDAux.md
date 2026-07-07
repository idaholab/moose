# ElementIDAux

!syntax description /AuxKernels/ElementIDAux

## Description

Returns the current element ID from the mesh.

## Example Input Syntax

Here is a minimal example showing how to declare `ElementIDAux`:

!listing test/tests/auxkernels/element_id_aux/element_id_aux.i
  block=AuxVariables AuxKernels

To determine the mesh elements corresponding to a given set of 3-D coordinates, 
`ElementIDAux` can be used in association with `PointValueSampler`:

!listing test/tests/auxkernels/element_id_aux/element_id_aux.i

This will return a list of element IDs corresponding to each location declared
in the sampler.

!syntax parameters /AuxKernels/ElementIDAux

!syntax inputs /AuxKernels/ElementIDAux

!syntax children /AuxKernels/ElementIDAux
