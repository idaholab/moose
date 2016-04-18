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
  params.addRequiredParam<AuxVariableName>("react_x", "The x reaction variable");
  params.addRequiredParam<AuxVariableName>("react_y", "The y reaction variable");
  params.addParam<AuxVariableName>("react_z", "The z reaction variable");

  params.addParam<RealVectorValue>("axis_origin", Point(), "Origin of the axis of rotation used to calculate the torque");
  params.addRequiredParam<RealVectorValue>("direction_vector", "The direction vector of the axis of rotation about which the calculated torque is calculated");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

TorqueReaction::TorqueReaction(const InputParameters & parameters) :
    NodalPostprocessor(parameters),
    _aux(_fe_problem.getAuxiliarySystem()),
    _react_x_var(_aux.getVariable(_tid, parameters.get<AuxVariableName>("react_x"))),
    _react_y_var(_aux.getVariable(_tid, parameters.get<AuxVariableName>("react_y"))),
    _react_z_var(isParamValid("react_z") ? &_aux.getVariable(_tid, parameters.get<AuxVariableName>("react_z")) : NULL),
    _react_x(_react_x_var.nodalSln()),
    _react_y(_react_y_var.nodalSln()),
    _react_z(_react_z_var ? _react_z_var->nodalSln() : _zero),
    _axis_origin(getParam<RealVectorValue>("axis_origin")),
    _direction_vector(getParam<RealVectorValue>("direction_vector"))
{
}

void
TorqueReaction::initialize()
{
  _sum = 0.0;
}

void
TorqueReaction::execute()
{
  // Tranform the node coordinates into the coordinate system specified by the user
  Point position = (*_current_node) - _axis_origin;

  // Determine the component of the vector in the direction of the rotation direction vector

  Point normal_position_component = position - (position * _direction_vector) / _direction_vector.norm_sq() * _direction_vector;

  // Define the force vector from the reaction force/ residuals from the stress divergence kernel
  Point force(_react_x[_qp], _react_y[_qp], _react_z[_qp]);

  // Cross the normal component of the position vector with the force
  RealVectorValue torque = normal_position_component.cross(force);

  // Find the component of the torque vector acting along the given axis of rotation direction vector
  RealVectorValue parallel_torque_component = (torque * _direction_vector) / _direction_vector.norm_sq() * _direction_vector;

  // Add the magnitude of the parallel torque component to the sum of the acting torques
  _sum += parallel_torque_component.norm();
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
