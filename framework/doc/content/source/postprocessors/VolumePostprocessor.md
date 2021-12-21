# VolumePostprocessor

!syntax description /Postprocessors/VolumePostprocessor

## Overview

Computes the volume of the Mesh or a subdomain of the Mesh.

## Example Input File Syntax

In this example, the volume of block 1, 2 and 3 are computed by three separate `VolumePostprocessor`.

!listing test/tests/postprocessors/volume/sphere1D.i block=Postprocessors

!syntax parameters /Postprocessors/VolumePostprocessor

!syntax inputs /Postprocessors/VolumePostprocessor

!syntax children /Postprocessors/VolumePostprocessor
