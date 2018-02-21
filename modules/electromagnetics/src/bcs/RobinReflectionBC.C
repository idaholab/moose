#include "RobinReflectionBC.h"
#include "Function.h"

template <>
InputParameters
validParams<RobinReflectionBC>()
{
  InputParameters params = validParams<IntegratedBC>();

  params.addRequiredParam<Real>("theta", "Incidence angle");
  params.addRequiredCoupledVar("coupled_field", "Coupled field variable");
  params.addRequiredParam<Real>("length", "length of slab");
  params.addRequiredParam<std::string>("num_type", "Real or imaginary variable type");
  params.addRequiredParam<Real>("k", "Wave number");
  params.addRequiredParam<FunctionName>("inverseMuR", "Inverse relative permeability");
  return params;
}

RobinReflectionBC::RobinReflectionBC(const InputParameters & parameters)
  : IntegratedBC(parameters),

    _theta(getParam<Real>("theta")),
    _coupled_val(coupledValue("coupled_field")),
    _L(getParam<Real>("length")),
    _num_type(getParam<std::string>("num_type")),
    _k(getParam<Real>("k")),
    _inverseMuR(getFunction("inverseMuR"))

{
}

Real
RobinReflectionBC::computeQpResidual()
{

  Real _coefficient = _k * std::cos(_theta * 2 * libMesh::pi / 360.);

  if (_num_type.compare("real") == 0)
  {
    return -_test[_i][_qp] * _inverseMuR.value(_t,_q_point[_qp]) * _coefficient * (_coupled_val[_qp] - 2 * std::sin(_k * _L * std::cos(_theta * 2 * libMesh::pi / 360.)));
  }
  else //if (_num_type.compare("imaginary") == 0)
  {
    return -_test[_i][_qp] * _inverseMuR.value(_t,_q_point[_qp]) * _coefficient * (2 * std::cos(_k * _L * std::cos(_theta * 2 * libMesh::pi / 360.)) - _coupled_val[_qp]);
  }
}
