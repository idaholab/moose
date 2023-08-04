# IntegralDirectedSurfaceForce

!syntax description /Postprocessors/IntegralDirectedSurfaceForce

## Explanation

This postprocessor which computes the directed force coming from friction and pressure
differences on a surface defined as:

!equation
F_d = \int\limits_S (\sigma \vec{n}) \cdot \vec{d} ~dS

where $\sigma$ is the Cauchy stress tensor. One can use this in combination with other postprocessors
to compute representative drag and lift coefficients.

## Example input syntax

In this case, the lift and drag coefficients are computed on the surface of a cylinder.

!listing examples/flow-over-circle/executioner_postprocessor.i block=Postprocessors

!syntax parameters /Postprocessors/IntegralDirectedSurfaceForce

!syntax inputs /Postprocessors/IntegralDirectedSurfaceForce

!syntax children /Postprocessors/IntegralDirectedSurfaceForce
