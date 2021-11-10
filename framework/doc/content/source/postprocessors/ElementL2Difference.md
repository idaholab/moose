# ElementL2Difference

!syntax description /Postprocessors/ElementL2Difference

The element $L_2$ difference between two variables $u$ and $v$ is computed as:

!equation
||u - v||_{L_2} = \sqrt{\sum_{\text{mesh elements}} \sum_{\text{quadrature point qp}} J[qp] W[qp] c[qp] (u(qp) - v(qp))^2}

The $J$ term accounts for the geometry/volume of the element, the $W$ for the weight in the [quadrature](syntax/Executioner/Quadrature/index.md),
$c$ for an eventual coordinate change (cylindrical, spherical) and $u(qp)$ and $v(qp)$ are the values of the variable at the quadrature point.

## Example input syntax

In this example, we compute the L2 difference between two variables that are solution of the same problem, but of different order, using `ElementL2Difference` postprocessors. $u$ is a linear lagrange variable while $v$ is a second order lagrange variable. We can verify by refining the mesh that their difference goes to 0.

!listing test/tests/postprocessors/element_l2_difference/element_l2_difference.i block=Variables Postprocessors

!syntax parameters /Postprocessors/ElementL2Difference

!syntax inputs /Postprocessors/ElementL2Difference

!syntax children /Postprocessors/ElementL2Difference
