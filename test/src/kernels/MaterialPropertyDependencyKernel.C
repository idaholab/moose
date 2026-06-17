#include "MaterialPropertyDependencyKernel.h"

registerMooseObject("MooseTestApp", MaterialPropertyDependencyKernel);

InputParameters
MaterialPropertyDependencyKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<MaterialPropertyName>("prop_name", "The material property to consume.");
  params.addParam<Real>(
      "dprop_dvar",
      1.0,
      "Derivative of the consumed material property with respect to the hidden coupled variable.");
  params.addClassDescription(
      "Test kernel for material-property variable dependency forwarding. The kernel consumes a "
      "material property but does not directly couple the variable used by the material.");
  return params;
}

MaterialPropertyDependencyKernel::MaterialPropertyDependencyKernel(
    const InputParameters & parameters)
  : Kernel(parameters),
    _prop(getMaterialProperty<Real>("prop_name")),
    _dprop_dvar(getParam<Real>("dprop_dvar"))
{
}

Real
MaterialPropertyDependencyKernel::computeQpResidual()
{
  return _prop[_qp] * _test[_i][_qp];
}

Real
MaterialPropertyDependencyKernel::computeQpJacobian()
{
  // The residual does not depend directly on this kernel's variable.
  return 0.0;
}

Real
MaterialPropertyDependencyKernel::computeQpOffDiagJacobian(unsigned int /* jvar */)
{
  // For the regression input, the only off-diagonal dependency should be the hidden material
  // dependency on v. If the framework does not forward that dependency, this method is never called.
  return _dprop_dvar * _phi[_j][_qp] * _test[_i][_qp];
}
