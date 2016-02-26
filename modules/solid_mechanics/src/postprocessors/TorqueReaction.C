/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "TorqueReaction.h"

template<>
InputParameters validParams<TorqueReaction>()
{
  InputParameters params = validParams<NodalPostprocessor>();
  params.addRequiredParam<AuxVariableName>("react_x","The x reaction variable");
  params.addRequiredParam<AuxVariableName>("react_y","The y reaction variable");
  params.addRequiredParam<AuxVariableName>("react_z","The z reaction variable");
  params.addRequiredParam<RealVectorValue>("axis_origin","Origin of the axis of rotation");
  params.addRequiredParam<RealVectorValue>("axis_direction","Direction of the axis of rotation");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

TorqueReaction::TorqueReaction(const InputParameters & parameters) :
    NodalPostprocessor(parameters),
    _aux(_fe_problem.getAuxiliarySystem()),
    _react_x_var(_aux.getVariable(_tid, parameters.get<AuxVariableName>("react_x"))),
    _react_y_var(_aux.getVariable(_tid, parameters.get<AuxVariableName>("react_y"))),
    _react_z_var(_aux.getVariable(_tid, parameters.get<AuxVariableName>("react_z"))),
    _react_x(_react_x_var.nodalSln()),
    _react_y(_react_y_var.nodalSln()),
    _react_z(_react_z_var.nodalSln()),
    _axis_origin(getParam<RealVectorValue>("axis_origin")),
    _axis_direction(getParam<RealVectorValue>("axis_direction")),
    _sum(0)
{
}

void
TorqueReaction::initialize()
{
  _sum = 0;
}

void
TorqueReaction::execute()
{
  //Calculate vector r from axis of rotation to node
  Point p(*_current_node);
  Point orig_to_p = p - _axis_origin;
  Real proj = orig_to_p * _axis_direction;
  Point proj_vec = proj * _axis_direction;
  Point r = orig_to_p - proj_vec;

  //Sum over T = r x F
  Point force(_react_x[_qp], _react_y[_qp], _react_z[_qp]);
  Point torque = r.cross(force);
  Real torque_len = torque.norm();
  _sum += torque_len;
}

Real
TorqueReaction::getValue()
{
  gatherSum(_sum);

  return _sum;
}

void
TorqueReaction::threadJoin(const UserObject & y)
{
  const TorqueReaction & pps = static_cast<const TorqueReaction &>(y);
  _sum += pps._sum;
}
