/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "LevelSetReinitializationProblem.h"

template<>
InputParameters validParams<LevelSetReinitializationProblem>()
{
  InputParameters params = validParams<FEProblem>();
  params.addClassDescription("A specialied problem that has a method for resetting time for level set reinitialization execution.");
  return params;

}

LevelSetReinitializationProblem::LevelSetReinitializationProblem(const InputParameters & parameters) :
    FEProblem(parameters)
{
}


void
LevelSetReinitializationProblem::resetTime()
{
  _time = 0.0;
  _time_old = 0.0;
  _t_step = 0;
  _termination_requested = false;
}
