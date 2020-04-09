//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrainForceAndTorqueInterface.h"
#include "GrainForcesPostprocessor.h"

registerMooseObject("PhaseFieldApp", GrainForcesPostprocessor);

InputParameters
GrainForcesPostprocessor::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription("Outputs the values from GrainForcesPostprocessor");
  params.addParam<UserObjectName>(
      "grain_force", "Specify userobject that gives center of mass and volume of grains");
  return params;
}

GrainForcesPostprocessor::GrainForcesPostprocessor(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _grain_force_torque_vector(declareVector("grain_force_torque_vector")),
    _grain_force_torque(getUserObject<GrainForceAndTorqueInterface>("grain_force")),
    _grain_forces(_grain_force_torque.getForceValues()),
    _grain_torques(_grain_force_torque.getTorqueValues()),
    _grain_num(0)
{
}

void
GrainForcesPostprocessor::initialize()
{
  _grain_num = _grain_forces.size();

  // for each grain a force and a torque vector with 3 components each are serialized into the
  // output vector
  _grain_force_torque_vector.resize(_grain_num * 2 * 3);
}

void
GrainForcesPostprocessor::execute()
{
  for (unsigned int i = 0; i < _grain_num; ++i)
  {
    _grain_force_torque_vector[6 * i + 0] = _grain_forces[i](0);
    _grain_force_torque_vector[6 * i + 1] = _grain_forces[i](1);
    _grain_force_torque_vector[6 * i + 2] = _grain_forces[i](2);
    _grain_force_torque_vector[6 * i + 3] = _grain_torques[i](0);
    _grain_force_torque_vector[6 * i + 4] = _grain_torques[i](1);
    _grain_force_torque_vector[6 * i + 5] = _grain_torques[i](2);
  }
}
