/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "GrainForceAndTorqueSum.h"

template<>
InputParameters validParams<GrainForceAndTorqueSum>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Userobject for summing forces and torques acting on a grain");
  params.addParam<std::vector<UserObjectName> >("grain_forces", "List of names of user objects that provides forces and torques applied to grains");
  params.addParam<unsigned int>("op_num", "Number of grains");
  return params;
}

GrainForceAndTorqueSum::GrainForceAndTorqueSum(const InputParameters & parameters) :
    GrainForceAndTorqueInterface(),
    GeneralUserObject(parameters),
    _sum_objects(getParam<std::vector<UserObjectName> >("grain_forces")),
    _num_forces(_sum_objects.size()),
    _ncrys(getParam<unsigned int>("op_num")),
    _sum_forces(_num_forces),
    _force_values(_ncrys),
    _torque_values(_ncrys),
    _force_derivatives(_ncrys),
    _torque_derivatives(_ncrys)

{
  for (unsigned int i = 0; i < _num_forces; ++i)
    _sum_forces[i] = & getUserObjectByName<GrainForceAndTorqueInterface>(_sum_objects[i]);
}

void
GrainForceAndTorqueSum::initialize()
{

  for (unsigned int i = 0; i < _ncrys; ++i)
  {
    _force_values[i] = 0.0;
    _torque_values[i] = 0.0;
    _force_derivatives[i] = 0.0;
    _torque_derivatives[i] = 0.0;

    for (unsigned int j = 0; j < _num_forces; ++j)
    {
      _force_values[i] += (_sum_forces[j]->getForceValues())[i];
      _torque_values[i] += (_sum_forces[j]->getTorqueValues())[i];
      _force_derivatives[i] += (_sum_forces[j]->getForceDerivatives())[i];
      _torque_derivatives[i] += (_sum_forces[j]->getTorqueDerivatives())[i];
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

const std::vector<RealGradient> &
GrainForceAndTorqueSum::getForceDerivatives() const
{
  return _force_derivatives;
}

const std::vector<RealGradient> &
GrainForceAndTorqueSum::getTorqueDerivatives() const
{
  return _torque_derivatives;
}
