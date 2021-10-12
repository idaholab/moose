# ElementH1SemiError

!syntax description /Postprocessors/ElementH1SemiError

The gradient of the variable is compared to the gradient of the specified function.
Examining the error on the gradient can reveal excessive oscillations, highlight
the need for a higher order finite element family, or show that the gradient
of a known solution is well captured by the numerical solution.

!equation
|u - f|_{H_1} = \left( \int_\Omega \left(\nabla u(x,t) - \nabla f(x,t) \right)^p d\Omega \right)^{\dfrac{1}{p}}

where $u$ is the `variable`, $f$ the `function`, $p$ the norm exponent and $\Omega$ the integration domain,
which may be limited to certain blocks using the `block` parameter.

## Example input syntax

In this example, we compute a variety of error with the H1, L2 and H1-semi norms to verify that the H1 error is the sum of the L2 error and the H1-semi error.

!listing test/tests/postprocessors/element_h1_error_pps/element_h1_error_pp_test.i block=Postprocessors

!syntax parameters /Postprocessors/ElementH1SemiError

!syntax inputs /Postprocessors/ElementH1SemiError

!syntax children /Postprocessors/ElementH1SemiError
