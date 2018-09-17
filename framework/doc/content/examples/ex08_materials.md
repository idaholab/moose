# Example 8 : Material Properties

[](---)

## Overview

- This is a convection-diffusion problem with a non-linear material property.
- The `ExampleDiffusion` kernel uses a coefficent produced from a linearly interpolated tabulated Material property.
- The `ExampleMaterial` object couples in the gradient of the "diffused" variable and uses it to make a gradient material property.
- The `ExampleConvection` kernel uses the gradient material property as a velocity vector.

[](---)

## Complete Source Files

[ex08.i](https://github.com/idaholab/moose/blob/devel/examples/ex08_materials/ex08.i)

[ExampleApp.C](https://github.com/idaholab/moose/blob/devel/examples/ex08_materials/src/base/ExampleApp.C)

## Example Material Object

[ExampleMaterial.h](https://github.com/idaholab/moose/blob/devel/examples/ex08_materials/include/materials/ExampleMaterial.h)

[ExampleMaterial.C](https://github.com/idaholab/moose/blob/devel/examples/ex08_materials/src/materials/ExampleMaterial.C)

[](---)

## Diffusion with Material Property

[ExampleDiffusion.h](https://github.com/idaholab/moose/blob/devel/examples/ex08_materials/include/kernels/ExampleDiffusion.h)

[ExampleDiffusion.C](https://github.com/idaholab/moose/blob/devel/examples/ex08_materials/src/kernels/ExampleDiffusion.C)

[](---)

## Convection with Material Property

[ExampleConvection.h](https://github.com/idaholab/moose/blob/devel/examples/ex08_materials/include/kernels/ExampleConvection.h)

[ExampleConvection.C](https://github.com/idaholab/moose/blob/devel/examples/ex08_materials/src/kernels/ExampleConvection.C)

[](---)

## Outputs

!media large_media/examples/ex08_convected.png
       caption=Convection
       style=width:50%;

!media large_media/examples/ex08_diffused.png
       caption=Diffusion
       style=width:50%;

