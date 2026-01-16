# FVGeometricAverage

## Overview

This object performs linear interpolation from cell centroids to a face using geometric
weights derived from the face location along the line connecting adjacent cell centroids. On
orthogonal meshes this yields second-order accurate face values for smooth fields; on general
unstructured meshes it provides a consistent baseline interpolation that is often used for
interpolating material coefficients (e.g., diffusion coefficients) to faces ([!cite](moukalled2016finite),
[!cite](jasak1996error)).

Let \(\phi_C\) and \(\phi_N\) be cell-centered values on the element and neighbor sides of a face.
Let \(g_C\in[0,1]\) denote the geometric weight associated with the element-side value (based on the
relative distances from the face to each centroid, measured along the centroid-to-centroid line).
The interpolated face value is

!equation
\phi_f = g_C\,\phi_C + (1-g_C)\,\phi_N.

This method also provides an advected interpolation handle with the same weights (independent of
flow direction). For advection, this corresponds to a centered/linear scheme and is not TVD in
general; use a limiter-based method when boundedness near sharp gradients is required (see
[Limiters](syntax/Limiters/index.md)).

## Example Syntax

Declare the interpolation method in `[InterpolationMethods]`:

!listing framework/doc/content/source/fvinterpolationmethods/examples/linear-fv-diffusion-coeff-geometric.i start=[geom] end=[] include-end=true

Use it for a coefficient functor via
[!param](/LinearFVKernels/LinearFVDiffusion/coeff_interp_method):

!listing framework/doc/content/source/fvinterpolationmethods/examples/linear-fv-diffusion-coeff-geometric.i start=[diffusion] end=[] include-end=true

## Useful Links

See [FVInterpolationMethod.md] for the base class, [Limiters](syntax/Limiters/index.md) for limiter
background, and [../linearfvkernels/LinearFVDiffusion.md] / [../linearfvkernels/LinearFVAdvection.md]
for typical usage.

!syntax parameters /InterpolationMethods/FVGeometricAverage

!syntax inputs /InterpolationMethods/FVGeometricAverage

!syntax children /InterpolationMethods/FVGeometricAverage

