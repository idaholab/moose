//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaskedGrainForceAndTorque.h"

registerMooseObject("PhaseFieldApp", MaskedGrainForceAndTorque);

InputParameters
MaskedGrainForceAndTorque::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Userobject for masking/pinning grains and making forces and torques "
                             "acting on that grain zero");
  params.addParam<UserObjectName>("grain_force",
                                  "userobject for getting force and torque acting on grains");
  params.addParam<std::vector<unsigned int>>("pinned_grains", "Grain numbers for pinned grains");
  return params;
}

MaskedGrainForceAndTorque::MaskedGrainForceAndTorque(const InputParameters & parameters)
  : GrainForceAndTorqueInterface(),
    GeneralUserObject(parameters),
    _grain_force_torque_input(getUserObject<GrainForceAndTorqueInterface>("grain_force")),
    _grain_forces_input(_grain_force_torque_input.getForceValues()),
    _grain_torques_input(_grain_force_torque_input.getTorqueValues()),
    _grain_force_c_jacobians_input(_grain_force_torque_input.getForceCJacobians()),
    _grain_force_eta_jacobians_input(_grain_force_torque_input.getForceEtaJacobians()),
    _pinned_grains(getParam<std::vector<unsigned int>>("pinned_grains")),
    _num_pinned_grains(_pinned_grains.size()),
    _grain_num(_grain_forces_input.size()),
    _force_values(_grain_num),
    _torque_values(_grain_num)
{
}

void
MaskedGrainForceAndTorque::initialize()
{
  for (unsigned int i = 0; i < _grain_num; ++i)
  {
    _force_values[i] = _grain_forces_input[i];
    _torque_values[i] = _grain_torques_input[i];

    if (_num_pinned_grains != 0)
    {
      for (unsigned int j = 0; j < _num_pinned_grains; ++j)
      {
        if (i == _pinned_grains[j])
        {
          _force_values[i] = 0.0;
          _torque_values[i] = 0.0;
        }
      }
    }
  }

  if (_fe_problem.currentlyComputingJacobian())
  {
    unsigned int total_dofs = _subproblem.es().n_dofs();
    _c_jacobians.resize(6 * _grain_num * total_dofs, 0.0);
    _eta_jacobians.resize(_grain_num);
    for (unsigned int i = 0; i < _grain_num; ++i)
      for (unsigned int j = 0; j < total_dofs; ++j)
      {
        _c_jacobians[(6 * i + 0) * total_dofs + j] =
            _grain_force_c_jacobians_input[(6 * i + 0) * total_dofs + j];
        _c_jacobians[(6 * i + 1) * total_dofs + j] =
            _grain_force_c_jacobians_input[(6 * i + 1) * total_dofs + j];
        _c_jacobians[(6 * i + 2) * total_dofs + j] =
            _grain_force_c_jacobians_input[(6 * i + 2) * total_dofs + j];
        _c_jacobians[(6 * i + 3) * total_dofs + j] =
            _grain_force_c_jacobians_input[(6 * i + 3) * total_dofs + j];
        _c_jacobians[(6 * i + 4) * total_dofs + j] =
            _grain_force_c_jacobians_input[(6 * i + 4) * total_dofs + j];
        _c_jacobians[(6 * i + 5) * total_dofs + j] =
            _grain_force_c_jacobians_input[(6 * i + 5) * total_dofs + j];

        if (_num_pinned_grains != 0)
          for (unsigned int k = 0; k < _num_pinned_grains; ++k)
            if (i == _pinned_grains[k])
            {
              _c_jacobians[(6 * i + 0) * total_dofs + j] = 0.0;
              _c_jacobians[(6 * i + 1) * total_dofs + j] = 0.0;
              _c_jacobians[(6 * i + 2) * total_dofs + j] = 0.0;
              _c_jacobians[(6 * i + 3) * total_dofs + j] = 0.0;
              _c_jacobians[(6 * i + 4) * total_dofs + j] = 0.0;
              _c_jacobians[(6 * i + 5) * total_dofs + j] = 0.0;
            }
      }

    for (unsigned int i = 0; i < _grain_num; ++i)
    {
      _eta_jacobians[i].resize(6 * _grain_num * total_dofs);
      for (unsigned int j = 0; j < _grain_num; ++j)
        for (unsigned int k = 0; k < total_dofs; ++k)
        {
          _eta_jacobians[i][(6 * j + 0) * total_dofs + k] =
              _grain_force_eta_jacobians_input[i][(6 * j + 0) * total_dofs + k];
          _eta_jacobians[i][(6 * j + 1) * total_dofs + k] =
              _grain_force_eta_jacobians_input[i][(6 * j + 1) * total_dofs + k];
          _eta_jacobians[i][(6 * j + 2) * total_dofs + k] =
              _grain_force_eta_jacobians_input[i][(6 * j + 2) * total_dofs + k];
          _eta_jacobians[i][(6 * j + 3) * total_dofs + k] =
              _grain_force_eta_jacobians_input[i][(6 * j + 3) * total_dofs + k];
          _eta_jacobians[i][(6 * j + 4) * total_dofs + k] =
              _grain_force_eta_jacobians_input[i][(6 * j + 4) * total_dofs + k];
          _eta_jacobians[i][(6 * j + 5) * total_dofs + k] =
              _grain_force_eta_jacobians_input[i][(6 * j + 5) * total_dofs + k];

          if (_num_pinned_grains != 0)
            for (unsigned int l = 0; l < _num_pinned_grains; ++l)
              if (j == _pinned_grains[l])
              {
                _eta_jacobians[i][(6 * j + 0) * total_dofs + k] = 0.0;
                _eta_jacobians[i][(6 * j + 1) * total_dofs + k] = 0.0;
                _eta_jacobians[i][(6 * j + 2) * total_dofs + k] = 0.0;
                _eta_jacobians[i][(6 * j + 3) * total_dofs + k] = 0.0;
                _eta_jacobians[i][(6 * j + 4) * total_dofs + k] = 0.0;
                _eta_jacobians[i][(6 * j + 5) * total_dofs + k] = 0.0;
              }
        }
    }
  }
}

const std::vector<RealGradient> &
MaskedGrainForceAndTorque::getForceValues() const
{
  return _force_values;
}

const std::vector<RealGradient> &
MaskedGrainForceAndTorque::getTorqueValues() const
{
  return _torque_values;
}

const std::vector<Real> &
MaskedGrainForceAndTorque::getForceCJacobians() const
{
  return _c_jacobians;
}

const std::vector<std::vector<Real>> &
MaskedGrainForceAndTorque::getForceEtaJacobians() const
{
  return _eta_jacobians;
}
