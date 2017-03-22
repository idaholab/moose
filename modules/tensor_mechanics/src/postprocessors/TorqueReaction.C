/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "TorqueReaction.h"

#include "AuxiliarySystem.h"

template <>
InputParameters
validParams<TorqueReaction>()
{
  InputParameters params = validParams<NodalPostprocessor>();
  params.addRequiredParam<std::vector<AuxVariableName>>("react", "The reaction variables");
  params.addParam<RealVectorValue>(
      "axis_origin", Point(), "Origin of the axis of rotation used to calculate the torque");
  params.addRequiredParam<RealVectorValue>("direction_vector",
                                           "The direction vector of the axis "
                                           "of rotation about which the "
                                           "calculated torque is calculated");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

TorqueReaction::TorqueReaction(const InputParameters & parameters)
  : NodalPostprocessor(parameters),
    _aux(_fe_problem.getAuxiliarySystem()),
    _axis_origin(getParam<RealVectorValue>("axis_origin")),
    _direction_vector(getParam<RealVectorValue>("direction_vector"))
{
  std::vector<AuxVariableName> reacts = getParam<std::vector<AuxVariableName>>("react");
  _nrt = reacts.size();

  for (unsigned int i = 0; i < _nrt; ++i)
    _react.push_back(&_aux.getVariable(_tid, reacts[i]).nodalSln());
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
  Point normal_position_component =
      position - (position * _direction_vector) / _direction_vector.norm_sq() * _direction_vector;

  // Define the force vector from the reaction force/ residuals from the stress divergence kernel
  Real _rz;
  if (_nrt == 3)
    _rz = (*_react[2])[_qp];
  else
    _rz = 0.0;

  Point force((*_react[0])[_qp], (*_react[1])[_qp], _rz);

  // Cross the normal component of the position vector with the force
  RealVectorValue torque = normal_position_component.cross(force);

  // Find the component of the torque vector acting along the given axis of rotation direction
  // vector
  RealVectorValue parallel_torque_component =
      (torque * _direction_vector) / _direction_vector.norm_sq() * _direction_vector;

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
