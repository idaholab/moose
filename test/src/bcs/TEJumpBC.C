#include "MooseSystem.h"
#include "TEJumpBC.h"
#include "ElementData.h"

#include "numeric_vector.h"

template<>
InputParameters validParams<TEJumpBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.addParam<Real>("value", 0.0, "The value the variable should have on the boundary");
  params.set<bool>("_integrated") = false;
  params.addParam<double>("t_jump", 1.0, "Time when the jump occurs");
  params.addParam<double>("slope", 1.0, "How steep the jump is");
  return params;
}

TEJumpBC::TEJumpBC(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, parameters),
   _t_jump(getParam<Real>("t_jump")),
   _slope(getParam<Real>("slope"))
{
}

Real
TEJumpBC::computeQpResidual()
{
  return _u[_qp] - (atan((_t - _t_jump)*libMesh::pi * _slope));
}
