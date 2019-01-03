# Jacobian Definition

## Using Automatic Differentiation

One can elect to sacrifice some computing speed and calculate Jacobians
automatically using
[automatic differentiation (AD)](https://en.wikipedia.org/wiki/Automatic_differentiation). MOOSE
employs the `DualNumber` class from the
[MetaPhysicL](https://github.com/roystgnr/MetaPhysicL) package in order to
enable AD. If the application developer wants to make use of AD, they should
inherit from `ADKernel` as opposed to `Kernel`. Additionally, when coupling in
variables, the `adCoupled*` methods should be used. For example, to retrieve a
coupled value, instead of using `coupledValue("v")` in the `ADKernel`
constructor, `adCoupledValue("v")` should be used. `adCoupledGradient` should
replace `coupledGradient`, etc. An example of coupling in an AD variable can be found in
[`ADCoupledConvection.C`](test/src/kernels/ADCoupledConvection.C) and
[`ADCoupledConvection.h`](test/include/kernels/ADCoupledConvection.h). Moreover,
material properties that may depend on the non-linear variables should be
retrieved using `getADMaterialProperty` instead of `getMaterialProperty`. They
should be declared in materials using `declareADProperty`. Example AD material
source and header files can be found
[here](test/src/materials/ADCoupledMaterial.C) and
[here](test/include/materials/ADCoupledMaterial.h); example kernel source and
header files that use AD material properties can be found
[here](test/src/kernels/ADMatDiffusionTest.C) and [here](test/include/kernels/ADMatDiffusionTest.h).

## Traditional Hand-coded Jacobians

Finite element shape functions are introduced in the documentation section
[shape functions](finite_element_concepts/shape_functions.md). There we outline
how our primary variables are summations of those shape functions multiplied by
constant coefficients which are our degrees of freedom. In
[numerical implementation](finite_element_concepts/numerical_implementation.md)
we give explicit illustration of how the derivative of a variable `u` with
respect to its jth degree of freedom ($$$u_j$$$) is equal to the jth shape function
$$$\phi_j$$$. Similarly the derivative of $$$\nabla u$$$ with respect to $$$u_j$$$ is
equal to $$$\nabla \phi_j$$$. The code expression  `_phi[_j][_qp]` represents
$$$\frac{\partial u}{\partial u_j}$$$ in any MOOSE framework residual and Jacobian
computing objects such as kernels and boundary conditions.

Any MOOSE kernel may have an arbitrary number of variables coupled into it. If
these coupled variables use the same shape function family and order, then their
associated $$$\phi_j$$$s will be equivalent. However, if `u` and `v` use different
shape functions then $$$\phi_{j,u} \ne \phi_{j,v}$$$. As a developer, however, you
do not ***in most cases*** have to worry about these differences in $$$\phi$$$. MOOSE automatically
updates the object member variable `_phi` to use the shape functions of the
variable for whom the Jacobian is currently being computed. ***However***, if
the primary variable `u` is a scalar-valued (single-component) finite element
variable and the coupled variable `v` is a vector-valued (multi-component)
finite element variable (or visa versa), then you must introduce an additional
member variable to represent the shape functions of the vector-valued
(scalar-valued) variable. The name of this variable is up to the developer, but
we suggest perhaps a `_standard_` prefix for scalar valued finite-element
variables and `_vector_` for vector valued finite-element variables. The
`_standard_` prefix is suggested over `_scalar_` so as not to be confused with a
`MooseVariableScalar`, which only has a single value over the entire spatial
domain. An example constructor for a standard kernel that couples in a
vector-valued FE variable is shown below:

```
EFieldAdvection::EFieldAdvection(const InputParameters & parameters)
  : Kernel(parameters),
    _efield_id(coupled("efield")),
    _efield(coupledVectorValue("efield")),
    _efield_var(*getVectorVar("efield", 0)),
    _vector_phi(_assembly.phi(_efield_var)),
    _mobility(getParam<Real>("mobility"))
{
}
```

The associated declarations are:

```
  const unsigned int _efield_id;
  const VectorVariableValue & _efield;
  VectorMooseVariable & _efield_var;
  const VectorVariablePhiValue & _vector_phi;
  const Real _mobility;
  Real _sgn;
```

Residual, on-diagonal, and off-diagonal methods are respectively

```
Real
EFieldAdvection::computeQpResidual()
{
  return -_grad_test[_i][_qp] * _sgn * _mobility * _efield[_qp] * _u[_qp];
}
```

and

```
Real
EFieldAdvection::computeQpJacobian()
{
  return -_grad_test[_i][_qp] * _sgn * _mobility * _efield[_qp] * _phi[_j][_qp];
}
```

and

```
Real
EFieldAdvection::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _efield_id)
    return -_grad_test[_i][_qp] * _sgn * _mobility * _vector_phi[_j][_qp] * _u[_qp];
  else
    return 0;
}

```
An example constructor for a vector kernel that couples in a
scalar-valued FE variable is shown below:

```
VectorCoupledGradientTimeDerivative::VectorCoupledGradientTimeDerivative(
    const InputParameters & parameters)
  : VectorKernel(parameters),
    _grad_v_dot(coupledGradientDot("v")),
    _d_grad_v_dot_dv(coupledDotDu("v")),
    _v_id(coupled("v")),
    _v_var(*getVar("v", 0)),
    _standard_grad_phi(_assembly.gradPhi(_v_var))
{
}

```
The associated declarations are:

```
  const VariableGradient & _grad_v_dot;
  const VariableValue & _d_grad_v_dot_dv;
  const unsigned _v_id;
  MooseVariable & _v_var;
  const VariablePhiGradient & _standard_grad_phi;
```

Residual and off-diagonal Jacobian methods are respectively:

```
Real
VectorCoupledGradientTimeDerivative::computeQpResidual()
{
  return _test[_i][_qp] * _grad_v_dot[_qp];
}
```

and

```
Real
VectorCoupledGradientTimeDerivative::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _v_id)
    return _test[_i][_qp] * _d_grad_v_dot_dv[_qp] * _standard_grad_phi[_j][_qp];

  else
    return 0.;
}
```

!alert note title=Flexibility
Note that only one member is needed to represent shape functions for standard
    `MooseVariable`s and `VectorMooseVariable`s. For example, if the vector-variables
    `v` and `w` are coupled into a standard kernel for `u`, only a single
    `_vector_phi` member needs to be added; there is not need for both a
    `_v_phi` and `_w_phi`. `_vector_phi` will be automatically updated to
    represent the shape functions for whichever vector variable the Jacobian is
    being computed for.
