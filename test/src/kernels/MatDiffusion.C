#include "MatDiffusion.h"

template<>
InputParameters validParams<MatDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<std::string>("prop_name", "the name of the material property we are going to use");

  MooseEnum prop_state("current, old, older", "current");
  params.addParam<MooseEnum>("prop_state", prop_state, "Declares which property state we should retrieve");
  return params;
}


MatDiffusion::MatDiffusion(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _prop_name(getParam<std::string>("prop_name"))
{
  MooseEnum prop_state = getParam<MooseEnum>("prop_state");

  if (prop_state == "current")
    _diff = &getMaterialProperty<Real>(_prop_name);
  else if (prop_state == "old")
    _diff = &getMaterialPropertyOld<Real>(_prop_name);
  else if (prop_state == "older")
    _diff = &getMaterialPropertyOlder<Real>(_prop_name);
}

Real
MatDiffusion::computeQpResidual()
{
  return (*_diff)[_qp] * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
MatDiffusion::computeQpJacobian()
{
  return (*_diff)[_qp] * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}
