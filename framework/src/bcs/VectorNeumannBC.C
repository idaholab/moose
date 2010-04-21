#include "VectorNeumannBC.h"

template<>
InputParameters validParams<VectorNeumannBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.set<Real>("value0")=0.0;
  params.set<Real>("value1")=0.0;
  params.set<Real>("value2")=0.0;
  return params;
}

VectorNeumannBC::VectorNeumannBC(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, setIntegratedParam(parameters, true))
  {
    _value(0)=_parameters.get<Real>("value0");
    _value(1)=_parameters.get<Real>("value1");
    _value(2)=_parameters.get<Real>("value2");
  }


Real
VectorNeumannBC::computeQpResidual()
  {
    return -_phi_face[_i][_qp]*(_value*_normals_face[_qp]);
  }

