/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "LevelSetCFLCondition.h"

template<>
InputParameters validParams<LevelSetCFLCondition>()
{
  InputParameters params = validParams<ElementPostprocessor>();
  params.addClassDescription("Compute the minimum timestep from the Courant–Friedrichs–Lewy (CFL) condition for the level-set equation.");
  params += validParams<LevelSetVelocityInterface<> >();
  return params;
}

LevelSetCFLCondition::LevelSetCFLCondition(const InputParameters & parameters) :
    LevelSetVelocityInterface<ElementPostprocessor>(parameters),
    _min_width(std::numeric_limits<Real>::max()),
    _max_velocity(std::numeric_limits<Real>::min())
{
}

void
LevelSetCFLCondition::execute()
{
  _min_width = std::min(_min_width, _current_elem->hmin());
  for (unsigned int qp = 0; qp < _q_point.size(); ++qp)
  {
    RealVectorValue vel(_velocity_x[qp], _velocity_y[qp], _velocity_z[qp]);
    _max_velocity = std::max(_max_velocity, std::abs(vel.size()));
  }
}

void
LevelSetCFLCondition::finalize()
{
  gatherMax(_max_velocity);
  gatherMin(_min_width);
}

void
LevelSetCFLCondition::threadJoin(const UserObject & user_object)
{
  const LevelSetCFLCondition & cfl = static_cast<const LevelSetCFLCondition&>(user_object);
  _min_width = std::min(_min_width, cfl._min_width);
  _max_velocity = std::max(_max_velocity, cfl._max_velocity);
}


PostprocessorValue
LevelSetCFLCondition::getValue()
{
  return _min_width / _max_velocity;
}
