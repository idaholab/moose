//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
      const auto & force_values = _sum_forces[j]->getForceValues();
      const auto & torque_values = _sum_forces[j]->getTorqueValues();

      if (force_values.size() != _grain_num || torque_values.size() != _grain_num)
        continue;

      _force_values[i] += force_values[i];
      _torque_values[i] += torque_values[i];
    }
  }

  if (_fe_problem.currentlyComputingJacobian())
  {
    unsigned int total_dofs = _subproblem.es().n_dofs();
    _c_jacobians.resize(6 * _grain_num * total_dofs, 0.0);
    _eta_jacobians.resize(_grain_num);

    for (unsigned int i = 0; i < _c_jacobians.size(); ++i)
      for (unsigned int j = 0; j < _num_forces; ++j)
      {
        const auto & c_jacobians = _sum_forces[j]->getForceCJacobians();
        if (c_jacobians.size() == _c_jacobians.size())
          _c_jacobians[i] += c_jacobians[i];
      }

    for (unsigned int i = 0; i < _grain_num; ++i)
    {
      _eta_jacobians[i].resize(6 * _grain_num * total_dofs, 0.0);
      for (unsigned int j = 0; j < _eta_jacobians[i].size(); ++j)
        for (unsigned int k = 0; k < _num_forces; ++k)
        {
          const auto & eta_jacobians = _sum_forces[k]->getForceEtaJacobians();
          if (eta_jacobians.size() == _grain_num &&
              eta_jacobians[i].size() == _eta_jacobians[i].size())
            _eta_jacobians[i][j] += eta_jacobians[i][j];
        }
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
