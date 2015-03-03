/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "SteadyTransientExecutioner.h"
#include "MooseApp.h"

template<>
InputParameters validParams<SteadyTransientExecutioner>()
{
  InputParameters params = validParams<CoupledExecutioner>();
  return params;
}

SteadyTransientExecutioner::SteadyTransientExecutioner(const std::string & name, InputParameters parameters) :
    CoupledExecutioner(name, parameters)
{
}

SteadyTransientExecutioner::~SteadyTransientExecutioner()
{
}

void
SteadyTransientExecutioner::execute()
{
  if (_executioners.size() < 2)
    mooseError("Not enough problems specified - need at least 2.");

  _executioners[0]->getMooseApp().setOutputWarehouse(_owhs[0]);
  _executioners[0]->init();
  _executioners[0]->execute();
  // project variables need by problem[1] into problem [1]
  projectVariables(*_fe_problems[1]);
  _executioners[1]->getMooseApp().setOutputWarehouse(_owhs[1]);
  _executioners[1]->init();
  _executioners[1]->execute();
}
