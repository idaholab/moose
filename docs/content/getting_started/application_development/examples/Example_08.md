# Material Properties

## Overview
---

- This is a convection-diffusion problem with a non-linear material property.
- The `ExampleDiffusion` kernel uses a coefficient produced from a linearly interpolated tabulated Material property.
- The `ExampleMaterial` object couples in the gradient of the "diffused" variable and uses it to make a gradient material property 
- The `ExampleConvection` kernel uses the gradient material property as a velocity vector.

## Complete Source Files
---
- [ex08.i](https://github.com/idaholab/moose/blob/devel/examples/ex08_materials/ex08.i)
- [ExampleApp.C](https://github.com/idaholab/moose/blob/devel/examples/ex08_materials/src/base/ExampleApp.C)

## Example Material Object
---

- [ExampleMaterial.h](https://github.com/idaholab/moose/blob/devel/examples/ex08_materials/include/materials/ExampleMaterial.h)
- [ExampleMaterial.C](https://github.com/idaholab/moose/blob/devel/examples/ex08_materials/src/materials/ExampleMaterial.C)

## Diffusion with Material Property
---

- [ExampleDiffusion.h](https://github.com/idaholab/moose/blob/devel/examples/ex08_materials/include/kernels/ExampleDiffusion.h)
- [ExampleDiffusion.C](https://github.com/idaholab/moose/blob/devel/examples/ex08_materials/src/kernels/ExampleDiffusion.C)

## Convection with Material Property
---

- [ExampleConvection.h](https://github.com/idaholab/moose/blob/devel/examples/ex08_materials/include/kernels/ExampleConvection.h)
- [ExampleConvection.C](https://github.com/idaholab/moose/blob/devel/examples/ex08_materials/src/kernels/ExampleConvection.C)

## Results
---

<!-- width at odd ammount to justify height with other image -->
!media media/examples/08_convected.png width=31.3% float=left caption=Convection

!media media/examples/08_diffused.png width=32% margin-left=1% float=left caption=Diffusion
