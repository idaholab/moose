# ElementSidesL2Norm

!syntax description /Postprocessors/ElementSidesL2Norm

The element-sides $L_2$ norm is defined as:

!equation
||u||_{L_2(\Gamma)} = \sqrt{\int_{\Gamma} u^2 d\Gamma}

Where here $\Gamma$ is defined as the union of all sides of all
elements in a mesh, counting internal sides shared by two elements
once rather than twice.

The side $L_2$ norm of a variable $u$ is computed as:

!equation
||u||_{L_2(\Gamma)} = \sqrt{\sum_{\text{mesh element sides}} \sum_{\text{quadrature point qp}} J[qp] W[qp] c[qp] u(qp)^2}

These terms are initialized on each side, so the $J$ term accounts
for the geometry/area of the element, the $W$ for the weight in the
[quadrature](syntax/Executioner/Quadrature/index.md), $c$ for any
eventual coordinate change (cylindrical, spherical) and $u(qp)$ is the
value of the variable at the quadrature point.

## Example input syntax

In this example, we compute the L2 norm for the Lagrange multiplier variable using an `ElementSideL2Norm` postprocessor.

!listing test/tests/outputs/exodus/exodus_side_discontinuous.i block=Postprocessors

!syntax parameters /Postprocessors/ElementSidesL2Norm

!syntax inputs /Postprocessors/ElementSidesL2Norm

!syntax children /Postprocessors/ElementSidesL2Norm
