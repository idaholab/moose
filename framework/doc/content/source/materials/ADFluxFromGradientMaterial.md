# ADFluxFromGradientMaterial

!syntax description /Materials/ADFluxFromGradientMaterial

## Overview

`ADFluxFromGradientMaterial` computes a vector-valued flux based on the gradient of a coupled variable and a scalar diffusivity material property.

It implements the relation:

\[
\mathbf{J} = -D \nabla u
\]

where:

- \( \mathbf{J} \) is the flux (vector)
- \( D \) is the scalar diffusivity (material property)
- \( u \) is the coupled variable

This flux can be used with the `ADFluxDivergence` kernel to model diffusion-like processes.

## Input Parameters

!syntax parameters /Materials/ADFluxFromGradientMaterial

## Example Input File

```ini
[Materials]
  [diffusivity]
    type = ADGenericConstantMaterial
    prop_names = 'diffusivity'
    prop_values = '1.0'
  []

  [flux]
    type = ADFluxFromGradientMaterial
    flux = flux
    u = u
    diffusivity = diffusivity
  []
[]
