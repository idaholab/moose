# QuadraturePointMultiApp

!syntax description /MultiApps/QuadraturePointMultiApp

## Description

Creates a sub-app at the quadrature points within every element in the parent app, which can be leveraged for
doing multiscale solves. This object requires no special parameters, but this is
[block restrictable](BlockRestrictable.md) so that the sub-applications can be restricted to only
be generated on specified subdomains.

!alert note
Quadrature points that are shared between elements are collapsed into the same sub-application. Please consider the
effect of this if working with variables that are discontinuous on element sides.

## Example Input Syntax

The following code snippet demonstrates the creation of a `QuadraturePointMultiApp` object.

!listing quadrature_point_multiapp/quadrature_point_multiapp.i block=MultiApps

!syntax parameters /MultiApps/QuadraturePointMultiApp

!syntax inputs /MultiApps/QuadraturePointMultiApp

!syntax children /MultiApps/QuadraturePointMultiApp
