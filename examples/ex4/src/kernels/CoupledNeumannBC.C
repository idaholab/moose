#include "CoupledNeumannBC.h"

template<>
InputParameters validParams<CoupledNeumannBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.addParam<Real>("value", 0.0, "Value multiplided by the coupled value on the boundary");
  return params;
}

CoupledNeumannBC::CoupledNeumannBC(std::string name, MooseSystem & moose_system, InputParameters parameters)
 :BoundaryCondition(name, moose_system, setIntegratedParam(parameters, true)),
  _value(_parameters.get<Real>("value")),
  _some_var_val(coupledVal("some_var"))
{}

Real
CoupledNeumannBC::computeQpResidual()
{
  return -_phi_face[_i][_qp]*_value*_some_var_val[_qp];
}
