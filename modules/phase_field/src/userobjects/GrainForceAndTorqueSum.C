//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrainForceAndTorqueSum.h"

registerMooseObject("PhaseFieldApp", GrainForceAndTorqueSum);

InputParameters
GrainForceAndTorqueSum::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Userobject for summing forces and torques acting on a grain");
  params.addParam<std::vector<UserObjectName>>(
      "grain_forces",
      "List of names of user objects that provides forces and torques applied to grains");
  params.addParam<unsigned int>("grain_num", "Number of grains");
  return params;
}

GrainForceAndTorqueSum::GrainForceAndTorqueSum(const InputParameters & parameters)
  : GrainForceAndTorqueInterface(),
    GeneralUserObject(parameters),
    _sum_objects(getParam<std::vector<UserObjectName>>("grain_forces")),
    _num_forces(_sum_objects.size()),
    _grain_num(getParam<unsigned int>("grain_num")),
    _sum_forces(_num_forces),
    _force_values(_grain_num),
    _torque_values(_grain_num)
{
  for (unsigned int i = 0; i < _num_forces; ++i)
    _sum_forces[i] = &getUserObjectByName<GrainForceAndTorqueInterface>(_sum_objects[i]);
}

void
GrainForceAndTorqueSum::initialize()
{
  for (unsigned int i = 0; i < _grain_num; ++i)
  {
    _force_values[i] = 0.0;
    _torque_values[i] = 0.0;
    for (unsigned int j = 0; j < _num_forces; ++j)
    {
      _force_values[i] += (_sum_forces[j]->getForceValues())[i];
      _torque_values[i] += (_sum_forces[j]->getTorqueValues())[i];
    }
  }

  if (_fe_problem.currentlyComputingJacobian())
  {
    unsigned int total_dofs = _subproblem.es().n_dofs();
    _c_jacobians.resize(6 * _grain_num * total_dofs, 0.0);
    _eta_jacobians.resize(_grain_num);

    for (unsigned int i = 0; i < _c_jacobians.size(); ++i)
      for (unsigned int j = 0; j < _num_forces; ++j)
        _c_jacobians[i] += (_sum_forces[j]->getForceCJacobians())[i];

    for (unsigned int i = 0; i < _grain_num; ++i)
    {
      _eta_jacobians[i].resize(6 * _grain_num * total_dofs, 0.0);
      for (unsigned int j = 0; j < _eta_jacobians[i].size(); ++j)
        for (unsigned int k = 0; k < _num_forces; ++k)
          _eta_jacobians[i][j] += (_sum_forces[k]->getForceEtaJacobians())[i][j];
    }
  }
}

const std::vector<RealGradient> &
GrainForceAndTorqueSum::getForceValues() const
{
  return _force_values;
}

const std::vector<RealGradient> &
GrainForceAndTorqueSum::getTorqueValues() const
{
  return _torque_values;
}

const std::vector<Real> &
GrainForceAndTorqueSum::getForceCJacobians() const
{
  return _c_jacobians;
}

const std::vector<std::vector<Real>> &
GrainForceAndTorqueSum::getForceEtaJacobians() const
{
  return _eta_jacobians;
}
