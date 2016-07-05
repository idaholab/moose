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
    // _grain_force_derivatives(_grain_force_torque.getForceJacobians()),
    // _grain_torque_derivatives(_grain_force_torque.getTorqueJacobians()),
    _total_grains(_grain_forces.size())
{
  _grain_force_torque_vector.resize(_total_grains*12);
}

void
GrainForcesPostprocessor::execute()
{
  for (unsigned int i=0; i< _total_grains; ++i)
  {
    _grain_force_torque_vector[12*i+0] = _grain_forces[i](0);
    _grain_force_torque_vector[12*i+1] = _grain_forces[i](1);
    _grain_force_torque_vector[12*i+2] = _grain_forces[i](2);
    _grain_force_torque_vector[12*i+3] = _grain_torques[i](0);
    _grain_force_torque_vector[12*i+4] = _grain_torques[i](1);
    _grain_force_torque_vector[12*i+5] = _grain_torques[i](2);

    // if (_fe_problem.currentlyComputingJacobian())
    // for (unsigned int j = 0; j < _subproblem.es().n_dofs(); ++j)
    // {
    //   _grain_force_torque_vector[12*i+6] += _grain_force_derivatives[i][j](0);
    //   _grain_force_torque_vector[12*i+7] += _grain_force_derivatives[i][j](1);
    //   _grain_force_torque_vector[12*i+8] += _grain_force_derivatives[i][j](2);
    //   _grain_force_torque_vector[12*i+9] += _grain_torque_derivatives[i][j](0);
    //   _grain_force_torque_vector[12*i+10] += _grain_torque_derivatives[i][j](1);
    //   _grain_force_torque_vector[12*i+11] += _grain_torque_derivatives[i][j](2);
    // }
  }
}
