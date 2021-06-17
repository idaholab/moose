# MooseVariableFVReal

!syntax description /Adaptivity/Markers/MooseVariableFVReal

## Overview

The `MooseVariableFV` template supports creation of finite volume variable
types. At the time of writing there is only one used instantiation of the
template which is `MooseVariableFVReal`. MOOSE uses cell-centered finite volumes
which can be conveniently supported by the `CONSTANT MONOMIAL` finite element
type in the background.

One important parameter for `MooseVariableFV` objects is
`two_term_boundary_expansion`. By default this is `false`. When `false`,
boundary faces which do not have associated
[Dirichlet boundary conditions](FVDirichletBC.md) simply use the cell centroid
value as the face value. However, when `two_term_boundary_expansion` is set to
`true` in the `Variables` sub-block of the finite volume variable, then the cell
centroid value *and* the reconstructed cell centroid gradient (hence the
two-term name) will be used to compute the extrapolated boundary face
value. Note that care should be taken when setting this parameter to
`true`. Solving for a two-term extrapolated boundary face value requires
simultaneously solving for the attached cell centroid gradient. This creates a
system of equations. This system has the potential to be singular if there are
multiple extrapolated boundary faces per cell; see
[MOOSE issue](https://github.com/idaholab/moose/issues/16822). The only time
it's reasonable to expect a cell with multiple extrapolated boundary faces to
yield a nonsingular system is if the vectors from the cell centroid to face
centroid are parallel to the surface normals *and* none of the surface normals
are parallel to themselves (e.g. imagine an orthogonal quad with three extrapolated
boundary faces).

Another parameter in the `MooseVariableFV` template is
`use_extended_stencil`. This is defaulted to `false`. When `false`, the gradient
is computed using face values which are linearly interpolated between the
current cell centroid and the neighboring cell centroids. When toggled to
`true`, the face values are computed as a weighted function of the attached face
vertices. This requires computing solution values at vertices which will be a
function of all adjacent cells. This extends the stencil and increases the size
of the Jacobian matrix. The benefit is supposed to be increased accuracy of the
gradient computation. However, with the current implementation the vertex
approach is actually *less* accurate (solution convergence of O(h^1.5) as
opposed to O(h^2) for the smaller stencil). This may indicate a bug in the
current vertex-based implementation which hopefully a smart user will come along
and fix.

## Example Input File Syntax

To create a `MooseVariableFVReal` a user can do one of the following in their
input file:

```
[Variables]
  [u]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
  [v]
    type = MooseVariableFVReal
  []
[]
```

Note that a user *must* specify the `type` if they want to be able to set finite
volume variable specific parameters like `two_term_boundary_expansion` or
`use_extended_stencil.`

!syntax parameters /Adaptivity/Markers/MooseVariableFVReal

!syntax inputs /Adaptivity/Markers/MooseVariableFVReal

!syntax children /Adaptivity/Markers/MooseVariableFVReal
