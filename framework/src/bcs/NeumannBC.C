#include "NeumannBC.h"

template<>
InputParameters validParams<NeumannBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.set<Real>("value")=0.0;
  return params;
}

NeumannBC::NeumannBC(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, setIntegratedParam(parameters, true)),
    _value(_parameters.get<Real>("value"))
 {}

Real
NeumannBC::computeQpResidual()
  {
    return -_phi_face[_i][_qp]*_value;
  }

