//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TorqueReaction.h"

// MOOSE includes
#include "AuxiliarySystem.h"
#include "MooseVariable.h"

registerMooseObject("TensorMechanicsApp", TorqueReaction);

InputParameters
TorqueReaction::validParams()
{
  InputParameters params = NodalPostprocessor::validParams();
  params.addClassDescription("TorqueReaction calculates the torque in 2D and 3D"
                             "about a user-specified axis of rotation centered"
                             "at a user-specified origin.");
  params.addRequiredParam<std::vector<AuxVariableName>>("reaction_force_variables",
                                                        "The reaction variables");
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
  std::vector<AuxVariableName> reacts =
      getParam<std::vector<AuxVariableName>>("reaction_force_variables");
  _nrt = reacts.size();

  for (unsigned int i = 0; i < _nrt; ++i)
    _react.push_back(&_aux.getFieldVariable<Real>(_tid, reacts[i]).dofValues());
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
  return _sum;
}

void
TorqueReaction::finalize()
{
  gatherSum(_sum);
}

void
TorqueReaction::threadJoin(const UserObject & y)
{
  const TorqueReaction & pps = static_cast<const TorqueReaction &>(y);
  _sum += pps._sum;
}
