#include "Reaction.h"

template<>
InputParameters validParams<Reaction>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

Reaction::Reaction(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters)
  {}

Real
Reaction::computeQpResidual()
  {
    return _test[_i][_qp]*_u[_qp];
  }

Real
Reaction::computeQpJacobian()
  {
    return _test[_i][_qp]*_phi[_j][_qp];
  }
