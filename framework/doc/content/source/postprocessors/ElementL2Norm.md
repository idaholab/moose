# ElementL2Norm

!syntax description /Postprocessors/ElementL2Norm

The element $L_2$ norm is defined as:

!equation
||u||_{L_2} = \sqrt{\int_{\Omega} u^2 d\Omega}

The element $L_2$ norm of a variable $u$ is computed as:

!equation
||u||_{L_2} = \sqrt{\sum_{\text{mesh elements}} \sum_{\text{quadrature point qp}} J[qp] W[qp] c[qp] u(qp)^2}

The $J$ term accounts for the geometry/volume of the element, the $W$ for the weight in the
[quadrature](syntax/Executioner/Quadrature/index.md), $c$ for an eventual coordinate change
(cylindrical, spherical) and $u(qp)$ is the value of the variable at the quadrature point.

## Example input syntax

In this example, we compute the L2 norm for a variety of auxiliary variable types using `ElementL2Norm` postprocessors.

!listing test/tests/auxkernels/element_aux_var/l2_element_aux_var_test.i block=Postprocessors

!syntax parameters /Postprocessors/ElementL2Norm

!syntax inputs /Postprocessors/ElementL2Norm

!syntax children /Postprocessors/ElementL2Norm
