#include "MTBC.h"

template<>
InputParameters validParams<MTBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<std::string>("prop_name", "the name of the material property we are going to use");
  params.addRequiredParam<Real>("grad", "the value of the gradient");
  return params;
}

MTBC::MTBC(const std::string & name, InputParameters parameters) :
    IntegratedBC(name, parameters),
    _value(getParam<Real>("grad")),
    _prop_name(getParam<std::string>("prop_name")),
    _mat(getMaterialProperty<Real>(_prop_name))
{
}

Real
MTBC::computeQpResidual()
{
  return -_test[_i][_qp]*_value*_mat[_qp];
}

