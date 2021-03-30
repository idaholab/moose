# FVInterfaceKernels System

For an overview of MOOSE FV please see [/fv_design.md].

`FVInterfaceKernels` are meant to communicate data at interfaces between
subdomains. An `FVInterfaceKernel` may contribute to the residuals/Jacobians of
a single variable, specified with the parameter `variable1`, or to multiple
variables by also using the `variable2` parameter. There are two additional
critical/required parameters: `subdomain1` and `subdomain2`. In cases for which an
`FVInterfaceKernel` is operating on two variables, `subdomain1` should
correspond to the subdomain(s) neighboring the `boundary` parameter that
`variable1` lives on, and similarly for `subdomain2` and `variable2`. By
checking the `subdomain` parameters against the subdomain IDs of the
`FaceInfo::elem` and `FaceInfo::neighbor` members a `FVInterfaceKernel`
developer can be sure that they are fetching and using sensical data. For
instance, a developer may want to create an `FVInterfaceKernel` that uses
`prop1` on the `subdomain1` side of the `boundary` and `prop2` on the
`subdomain2` side of the boundary. However, MOOSE only provides these APIs for
fetching material properties: `get(AD)MaterialProperty` and
`getNeighbor(AD)MaterialProperty`. The return value of `get(AD)MaterialProperty`
will always correspond to a material property evaluation on the `FaceInfo::elem`
side of a (inter)face, while the return value of
`getNeighbor(AD)MaterialProperty` will always correspond to a material property
evaluation on the `FaceInfo::neighbor` side of a (inter)face. However, when
moving along an interface, it is possible that the `FaceInfo::elem` side of the
interface is sometimes the `subdomain1` side and sometimes the `subdomain2`
side. So making use of the `subdomain` parameters, we provide a protected method
called `elemIsOne()` that returns a boolean indicating whether the
`FaceInfo::elem` side of the interface corresponds to the `subdomain1` side of
the interface. This allows the developer to write code like the following:
```
FVFooInterface::FVFooInterface(const InputParameters & params)
  : FVInterfaceKernel(params),
    _coeff1_elem(getADMaterialProperty<Real>("coeff1")),
    _coeff2_elem(getADMaterialProperty<Real>("coeff2")),
    _coeff1_neighbor(getNeighborADMaterialProperty<Real>("coeff1")),
    _coeff2_neighbor(getNeighborADMaterialProperty<Real>("coeff2"))
{
}

ADReal
FVFooInterface::computeQpResidual()
{
  const auto & coef_elem = elemIsOne() ? _coeff1_elem : _coeff2_elem;
  const auto & coef_neighbor = elemIsOne() ? _coeff2_neighbor : _coeff1_neighbor;

  /// Code that uses coef_elem and coef_neighbor
}
```
and have confidence that they have good data in `coef_elem` and `coef_neighbor`
and have clarity about what is happening in their code.
