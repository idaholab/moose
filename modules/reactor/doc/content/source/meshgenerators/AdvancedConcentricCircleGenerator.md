# AdvancedConcentricCircleGenerator

!syntax description /Mesh/AdvancedConcentricCircleGenerator

## Overview

`AdvancedConcentricCircleGenerator` generates concentric circular meshes that are similar to the output of [`PolygonConcentricCircleMeshGenerator`](PolygonConcentricCircleMeshGenerator.md) but without background and duct regions. While all the features of `PolygonConcentricCircleMeshGenerator` involving the concentric circular "ring" regions are preserved, this mesh generator has some unique features because of the absence of the polygon outer regions.

### Azimuthal Mesh Density

By default, the azimuthal intervals are uniform and a single parameter [!param](/Mesh/AdvancedConcentricCircleGenerator/num_sectors) is used to define the total number of the azimuthal intervals.

Users can alternatively provide a list of ascending azimuthal angles between 0 to 360$^{\circ}$ through [!param](/Mesh/AdvancedConcentricCircleGenerator/customized_azimuthal_angles), which correspond to node angular positions of the generated mesh. Any azimuthal interval widths less than 120$^{\circ}$ are supported.

Due to the extra flexibility in azimuthal mesh density, the central elements have to be triangular.

## Example Syntax

!listing modules/reactor/test/tests/meshgenerators/advanced_concentric_circle_generator/accg.i block=Mesh

!media reactor/meshgenerators/accg_uniform.png
      style=display: block;margin-left:auto;margin-right:auto;width:40%;
      id=accg_uniform
      caption=An example output of `AdvancedConcentricCircleGenerator` with (8) uniform azimuthal sectors.

!media reactor/meshgenerators/accg_azi.png
      style=display: block;margin-left:auto;margin-right:auto;width:40%;
      id=accg_azi
      caption=An example output of `AdvancedConcentricCircleGenerator` with (8) customized azimuthal sectors.

!syntax parameters /Mesh/AdvancedConcentricCircleGenerator

!syntax inputs /Mesh/AdvancedConcentricCircleGenerator

!syntax children /Mesh/AdvancedConcentricCircleGenerator
