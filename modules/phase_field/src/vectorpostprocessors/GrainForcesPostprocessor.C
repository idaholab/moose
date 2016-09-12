/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GrainForcesPostprocessor.h"
#include "GrainForceAndTorqueInterface.h"

template<>
InputParameters validParams<GrainForcesPostprocessor>()
{
  InputParameters params = validParams<VectorPostprocessor>();
  params.addClassDescription("Outputs the values from GrainForcesPostprocessor");
  params.addParam<UserObjectName>("grain_force", "Specify userobject that gives center of mass and volume of grains");
  return params;
}

GrainForcesPostprocessor::GrainForcesPostprocessor(const InputParameters & parameters) :
    GeneralVectorPostprocessor(parameters),
    _grain_force_torque_vector(declareVector("grain_force_torque_vector")),
    _grain_force_torque(getUserObject<GrainForceAndTorqueInterface>("grain_force")),
    _grain_forces(_grain_force_torque.getForceValues()),
    _grain_torques(_grain_force_torque.getTorqueValues()),
    _grain_num(_grain_forces.size())
{
  _grain_force_torque_vector.resize(_grain_num*12);
}

void
GrainForcesPostprocessor::execute()
{
  for (unsigned int i=0; i< _grain_num; ++i)
  {
    _grain_force_torque_vector[12*i+0] = _grain_forces[i](0);
    _grain_force_torque_vector[12*i+1] = _grain_forces[i](1);
    _grain_force_torque_vector[12*i+2] = _grain_forces[i](2);
    _grain_force_torque_vector[12*i+3] = _grain_torques[i](0);
    _grain_force_torque_vector[12*i+4] = _grain_torques[i](1);
    _grain_force_torque_vector[12*i+5] = _grain_torques[i](2);
  }
}
