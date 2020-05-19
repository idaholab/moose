# PhiZero

MOOSE has helper zero objects for shape functions and shape function gradients that can be accessed by all MOOSE objects. The names of the zero objects are: `_phi_zero`, `_grad_phi_zero` and `_second_phi_zero`.
These object first two dimensions are: (i) the maximum number of degree of freedom per variable per element, and (ii) the maximum number of quadrature points per element.
The `PhiZeroKernel` test object checks that the first two dimensions of `_phi_zero`, `_grad_phi_zero` and `_second_phi_zero` are
consistent with the maximum number of shape functions and quadrature points in the simulation.
