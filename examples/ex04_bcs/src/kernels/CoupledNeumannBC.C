#include "CoupledNeumannBC.h"

template<>
InputParameters validParams<CoupledNeumannBC>()
{
  InputParameters params = validParams<BoundaryCondition>();

  // Here we are adding a parameter that will be extracted from the input file by the Parser
  params.addParam<Real>("value", 0.0, "Value multiplided by the coupled value on the boundary");
  params.addRequiredCoupledVar("some_var", "Flux Value at the Boundary");
  return params;
}

CoupledNeumannBC::CoupledNeumannBC(std::string name, MooseSystem & moose_system, InputParameters parameters)
 :BoundaryCondition(name, moose_system, parameters),
  _value(_parameters.get<Real>("value")),
  _some_var_val(coupledVal("some_var"))
{}

Real
CoupledNeumannBC::computeQpResidual()
{
  return -_phi[_i][_qp]*_value*_some_var_val[_qp];
}
