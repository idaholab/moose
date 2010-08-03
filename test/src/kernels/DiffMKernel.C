#include "DiffMKernel.h"

template<>
InputParameters validParams<DiffMKernel>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<std::string>("mat_prop", "the name of the material property we are going to use");
  return params;
}


DiffMKernel::DiffMKernel(std::string name, MooseSystem & moose_system, InputParameters parameters)
  : Kernel(name, moose_system, parameters),
    _prop_name(parameters.get<std::string>("mat_prop")),
    _diff(getMaterialProperty<Real>(_prop_name)),
    _vec_prop(getMaterialProperty<MooseArray<Real> >("vector_property"))
{
}

Real
DiffMKernel::computeQpResidual()
{
  return _diff[_qp] * _grad_test[_i][_qp] * _grad_u[_qp] - 4.0;
}

Real
DiffMKernel::computeQpJacobian()
{
  return _diff[_qp] * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}
