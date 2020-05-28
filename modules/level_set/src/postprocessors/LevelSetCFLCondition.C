//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetCFLCondition.h"

registerMooseObject("LevelSetApp", LevelSetCFLCondition);

InputParameters
LevelSetCFLCondition::validParams()
{
  InputParameters params = ElementPostprocessor::validParams();
  params.addClassDescription("Compute the minimum timestep from the Courant-Friedrichs-Lewy (CFL) "
                             "condition for the level-set equation.");
  params.addRequiredCoupledVar("velocity", "Velocity vector variable.");
  return params;
}

LevelSetCFLCondition::LevelSetCFLCondition(const InputParameters & parameters)
  : ElementPostprocessor(parameters),
    _cfl_timestep(std::numeric_limits<Real>::max()),
    _velocity(adCoupledVectorValue("velocity"))
{
}

void
LevelSetCFLCondition::execute()
{
  // Compute maximum velocity
  _max_velocity = std::numeric_limits<Real>::min();
  for (unsigned int qp = 0; qp < _q_point.size(); ++qp)
  {
    RealVectorValue vel = MetaPhysicL::raw_value(_velocity[qp]);
    _max_velocity = std::max(_max_velocity, std::abs(vel.norm()));
  }
  _cfl_timestep = std::min(_cfl_timestep, _current_elem->hmin() / _max_velocity);
}

void
LevelSetCFLCondition::finalize()
{
  gatherMin(_cfl_timestep);
}

void
LevelSetCFLCondition::threadJoin(const UserObject & user_object)
{
  const LevelSetCFLCondition & cfl = static_cast<const LevelSetCFLCondition &>(user_object);
  _cfl_timestep = std::min(_cfl_timestep, cfl._cfl_timestep);
}

PostprocessorValue
LevelSetCFLCondition::getValue()
{
  return _cfl_timestep;
}
