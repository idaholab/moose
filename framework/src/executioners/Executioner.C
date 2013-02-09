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

#include "Executioner.h"

// Moose includes
#include "MooseMesh.h"
#include "FEProblem.h"

// C++ includes
#include <vector>
#include <limits>


template<>
InputParameters validParams<Executioner>()
{
  InputParameters params = validParams<MooseObject>();
  params.addParam<std::string>("restart_file_base", "File base name used for restart");

  params.addPrivateParam<std::string>("built_by_action", "setup_executioner");

  params.addParamNamesToGroup("restart_file_base", "Restart");

  return params;
}

Executioner::Executioner(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    UserObjectInterface(parameters),
    PostprocessorInterface(parameters),
    _initial_residual_norm(std::numeric_limits<Real>::max()),
    _old_initial_residual_norm(std::numeric_limits<Real>::max()),
    _restart_file_base(getParam<std::string>("restart_file_base"))
{
}

Executioner::~Executioner()
{
}

void
Executioner::preExecute()
{
}

void
Executioner::postExecute()
{
}

void
Executioner::preSolve()
{
}

void
Executioner::postSolve()
{
}
