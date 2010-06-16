#include "NeumannBC.h"

template<>
InputParameters validParams<NeumannBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.addParam<Real>("value", 0.0, "The value of the gradient on the boundary.");
  return params;
}

NeumannBC::NeumannBC(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, parameters),
    _value(_parameters.get<Real>("value"))
 {}

Real
NeumannBC::computeQpResidual()
  {
    return -_phi[_i][_qp]*_value;
  }

