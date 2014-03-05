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
#include "MooseApp.h"

// C++ includes
#include <vector>
#include <limits>


template<>
InputParameters validParams<Executioner>()
{
  InputParameters params = validParams<MooseObject>();
  params.addParam<FileNameNoExtension>("restart_file_base", "File base name used for restart");

  params.registerBase("Executioner");

  params.addParamNamesToGroup("restart_file_base", "Restart");

  params.addParam<std::vector<std::string> >("splitting","Top-level splitting defining a hierarchical decomposition into subsystems to help the solver.");

  return params;
}

Executioner::Executioner(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    UserObjectInterface(parameters),
    PostprocessorInterface(parameters),
    Restartable(name, parameters, "Executioners"),
    _output_initial(false),
    _initial_residual_norm(std::numeric_limits<Real>::max()),
    _old_initial_residual_norm(std::numeric_limits<Real>::max()),
    _restart_file_base(getParam<FileNameNoExtension>("restart_file_base")),
    _splitting(getParam<std::vector<std::string> >("splitting"))
{
}

Executioner::~Executioner()
{
}

void
Executioner::init()
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

void
Executioner::outputInitial(bool out_init)
{
  _output_initial = out_init;
}
