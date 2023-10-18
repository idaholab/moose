# SelfShadowSideUserObject

!syntax description /UserObjects/SelfShadowSideUserObject

## Description

`SelfShadowSideUserObject` determines the illumination state (illuminated or in the shadow) of every quadrature point in a given sideset (or combination of sidesets). It is used by
[DirectionalFluxBC](DirectionalFluxBC.md).

!listing directional_flux_bc/2d.i block=UserObjects/shadow

## Design

The side user object is executed on every side of the sideset. A list of processor local quadrature points for each element/side combination is compiled, and each side is decomposes in either line segments (in 2D) or triangles (in 3D). The lists of line segments or triangles are broadcast to all processors.

The coordinates in both lists are rotated so that the illumination direction is aligned with the x-axis (2D) or z-axis (3D), which simplifies the shadowing calculation. Each processor then loops over the list of processor local quadrature points and checks each if they are in the shadow of any line segment or triangle.

!syntax parameters /UserObjects/SelfShadowSideUserObject

!syntax inputs /UserObjects/SelfShadowSideUserObject

!syntax children /UserObjects/SelfShadowSideUserObject

!bibtex bibliography
