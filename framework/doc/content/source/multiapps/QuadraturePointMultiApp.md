# QuadraturePointMultiApp

!syntax description /MultiApps/QuadraturePointMultiApp

## Description

Creates a sub-app at the quadrature points within every element in the parent app, which can be leveraged for
doing multiscale solves. This object requires no special parameters, but this is
[block restrictable](BlockRestrictable.md) so that the sub-applications can be restricted to only
be generated on specified subdomains.

!alert note
Boundary quadrature points are not added to the list of quadrature points, only element quadrature points.

!alert warning
The default mesh quadrature order will be used to locate quadrature points, not the actual quadrature order from the
[Quadrature](syntax/Executioner/Quadrature/index.md) specified in the Executioner.

## Example Input Syntax

The following code snippet demonstrates the creation of a `QuadraturePointMultiApp` object.

!listing quadrature_point_multiapp/quadrature_point_multiapp.i block=MultiApps

!syntax parameters /MultiApps/QuadraturePointMultiApp

!syntax inputs /MultiApps/QuadraturePointMultiApp

!syntax children /MultiApps/QuadraturePointMultiApp
