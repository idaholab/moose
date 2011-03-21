#include "TEJumpBC.h"

#include "numeric_vector.h"

template<>
InputParameters validParams<TEJumpBC>()
{
  InputParameters params = validParams<NodalBC>();
  params.addParam<Real>("value", 0.0, "The value the variable should have on the boundary");
  params.addParam<double>("t_jump", 1.0, "Time when the jump occurs");
  params.addParam<double>("slope", 1.0, "How steep the jump is");
  return params;
}

TEJumpBC::TEJumpBC(const std::string & name, InputParameters parameters) :
    NodalBC(name, parameters),
    _t_jump(getParam<Real>("t_jump")),
    _slope(getParam<Real>("slope"))
{
}

Real
TEJumpBC::computeQpResidual()
{
  return _u[_qp] - (atan((_t - _t_jump)*libMesh::pi * _slope));
}
