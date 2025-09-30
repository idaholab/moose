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

## Example Input File

!listing test/tests/kernels/ad_flux_divergence/1d_fluxdivergence_steadystate_test.i block= Materials/flux

!syntax parameters /Materials/ADFluxFromGradientMaterial

!syntax inputs /Materials/ADFluxFromGradientMaterial

!syntax children /Materials/ADFluxFromGradientMaterial
