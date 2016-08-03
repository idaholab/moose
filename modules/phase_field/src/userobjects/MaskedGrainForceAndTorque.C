/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "MaskedGrainForceAndTorque.h"

template<>
InputParameters validParams<MaskedGrainForceAndTorque>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Userobject for masking/pinning grains and making forces and torques acting on that grain zero");
  params.addParam<UserObjectName>("grain_force", "userobject for getting force and torque acting on grains");
  params.addParam<std::vector<unsigned int> >("pinned_grains", "Grain numbers for pinned grains");
  return params;
}

MaskedGrainForceAndTorque::MaskedGrainForceAndTorque(const InputParameters & parameters) :
    GrainForceAndTorqueInterface(),
    GeneralUserObject(parameters),
    _grain_force_torque_input(getUserObject<GrainForceAndTorqueInterface>("grain_force")),
    _grain_forces_input(_grain_force_torque_input.getForceValues()),
    _grain_torques_input(_grain_force_torque_input.getTorqueValues()),
    _grain_force_derivatives_input(_grain_force_torque_input.getForceDerivatives()),
    _grain_torque_derivatives_input(_grain_force_torque_input.getTorqueDerivatives()),
    _pinned_grains(getParam<std::vector<unsigned int> >("pinned_grains")),
    _num_pinned_grains(_pinned_grains.size()),
    _ncrys(_grain_forces_input.size()),
    _force_values(_ncrys),
    _torque_values(_ncrys),
    _force_derivatives(_ncrys),
    _torque_derivatives(_ncrys)
{
}

void
MaskedGrainForceAndTorque::initialize()
{
  for (unsigned int i = 0; i < _ncrys; ++i)
  {
    _force_values[i] = _grain_forces_input [i];
    _torque_values[i] = _grain_torques_input[i];
    _force_derivatives[i] = _grain_force_derivatives_input[i];
    _torque_derivatives[i] = _grain_torque_derivatives_input[i];

    if (_num_pinned_grains != 0)
    {
      for (unsigned int j = 0; j < _num_pinned_grains; ++j)
      {
        if (i == _pinned_grains[j])
        {
          _force_values[i] = 0.0;
          _torque_values[i] = 0.0;
          _force_derivatives[i] = 0.0;
          _torque_derivatives[i] = 0.0;
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

const std::vector<RealGradient> &
MaskedGrainForceAndTorque::getForceDerivatives() const
{
  return _force_derivatives;
}

const std::vector<RealGradient> &
MaskedGrainForceAndTorque::getTorqueDerivatives() const
{
  return _torque_derivatives;
}
