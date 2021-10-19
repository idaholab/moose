# ElementH1Error

!syntax description /Postprocessors/ElementH1Error

The H1 error is a popular metric of error as it is associated with the natural norm
for commonly used finite element spaces.

!equation
||u - f||_{H_1} = \left( \int_\Omega \left( u(x,t) - f(x,t) \right)^p + \left(\nabla u(x,t) - \nabla f(x,t) \right)^p d\Omega \right)^{\dfrac{1}{p}}

where `u` is the variable parameter, `f` the function parameter, $p$ the norm exponent and $\Omega$ the domain of integration,
which may be limited to certain blocks using the `block` parameter.

## Example input syntax

In this example, we compute a variety of error between a variable and a function with the H1, L2 and H1-semi norms to verify that the H1 error is the sum of the L2 error and the H1-semi error.

!listing postprocessors/element_h1_error_pps/element_h1_error_pp_test.i block=Postprocessors

!syntax parameters /Postprocessors/ElementH1Error

!syntax inputs /Postprocessors/ElementH1Error

!syntax children /Postprocessors/ElementH1Error
