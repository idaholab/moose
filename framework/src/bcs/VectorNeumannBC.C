#include "VectorNeumannBC.h"

template<>
InputParameters validParams<VectorNeumannBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.addParam<Real>("value0", 0.0, "x component of the vector this BC should act in.");
  params.addParam<Real>("value1", 0.0, "y component of the vector this BC should act in.");
  params.addParam<Real>("value2", 0.0, "z component of the vector this BC should act in.");
  return params;
}

VectorNeumannBC::VectorNeumannBC(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, parameters)
  {
    _value(0)=getParam<Real>("value0");
    _value(1)=getParam<Real>("value1");
    _value(2)=getParam<Real>("value2");
  }


Real
VectorNeumannBC::computeQpResidual()
  {
    return -_phi[_i][_qp]*(_value*_normals[_qp]);
  }

