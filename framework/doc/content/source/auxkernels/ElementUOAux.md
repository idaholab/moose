# ElementUOAux

!syntax description /AuxKernels/ElementUOAux

## Overview

An aux kernel is used to retrieve values from user objects. For example, it can be
combined with [ElemSideNeighborLayersGeomTester](ElemSideNeighborLayersGeomTester.md)
 or [ElemSideNeighborLayersTester](ElemSideNeighborLayersTester.md) to output
 ghosting information.

## Example syntax

In this example, the `ElementUOAux` is used to examine the ghosting of elements on various ranks, computed by
the three `ElemSideNeighborLayersGeomTester` on each rank.

!listing test/tests/mesh/uniform_refine/3d_diffusion.i block=UserObjects AuxKernels

!syntax parameters /AuxKernels/ElementUOAux

!syntax inputs /AuxKernels/ElementUOAux

!syntax children /AuxKernels/ElementUOAux
