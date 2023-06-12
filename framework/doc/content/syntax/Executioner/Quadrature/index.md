# Quadrature System

The `Quadrature` is used to integrate numerically in the domain and on sides. Some examples of integrals performed
by MOOSE are the integration of the residual over an element, or the postprocessing of spatial variable integrals / averages
over the domain.

The `Quadrature` order is chosen by default so that the product of the test function and the shape functions is integrated exactly.
When using Galerkin's method, this would mean the quadrature needs to integrate exactly polynomials of order
twice the order of the variables. In order to be able to integrate exactly with a potential third multiplied term in the definition of the residual, we
actually use a quadrature of order $2 n + 1$, with n the order of the variables.

In 1D, the following quadratures are known to integrate exactly:

- GAUSS (-Legendre), the default rule, with n points integrates exactly 1D polynomials of order up to 2n-1
- GAUSS_LOBATTO with n points integrates exactly 1D polynomials of order up to 2n-3
- SIMPSON's rule is known to integrate exactly 1D polynomials of order up to 3

In higher dimensions, the quadratures are defined differently based on the element types.
The user is referred to the libmesh doxygen for each quadrature rule object. They generally may be either:

- tensor product of 1D quadratures to be able to able to integrate exactly $x^a y^b z^c$ terms
  with $a\leq p, b\leq p, c\leq p$ with a quadrature of order $p$.
- using Dunavant's rule for quadratures on triangle elements
- extracted from the finite element literature


!alert note
Due to a current issue in the documentation system, the `type` parameter of the quadrature does not
display in the list of parameters. The available options are `CLOUGH CONICAL GAUSS GRID MONOMIAL SIMPSON TRAP GAUSS_LOBATTO`
with a default of `GAUSS` (-Legendre).

## Lowering the quadrature order

The quadrature order may be lowered intentionally for various purposes.
The side integration quadrature can be set at different order than the element integration one, using the
[!param](/Executioner/Quadrature/SetupQuadratureAction/element_order) and [!param](/Executioner/Quadrature/SetupQuadratureAction/side_order) parameters.
Custom quadrature orders may be set on selected blocks using the [!param](/Executioner/Quadrature/SetupQuadratureAction/custom_blocks)
and [!param](/Executioner/Quadrature/SetupQuadratureAction/custom_orders) parameters

## Example syntax

In this example, we specify a different quadrature rule using the `[Quadrature]` block
nested in the `[Executioner]` block.

!listing tests/quadrature/gauss_lobatto/gauss_lobatto.i block=Executioner

!syntax list /Executioner/Quadrature objects=True actions=False subsystems=False

!syntax list /Executioner/Quadrature objects=False actions=False subsystems=True

!syntax list /Executioner/Quadrature objects=False actions=True subsystems=False
