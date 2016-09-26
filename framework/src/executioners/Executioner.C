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

// Moose includes
#include "Executioner.h"
#include "MooseMesh.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "NonlinearSystem.h"

// C++ includes
#include <vector>
#include <limits>

template<>
InputParameters validParams<Executioner>()
{
  InputParameters params = validParams<MooseObject>();
  params.registerBase("Executioner");
  return params;
}

Executioner::Executioner(const InputParameters & parameters) :
    MooseObject(parameters),
    UserObjectInterface(this),
    PostprocessorInterface(this),
    Restartable(parameters, "Executioners"),
    _fe_problem(*parameters.getCheckedPointerParam<FEProblem *>("_fe_problem", "This might happen if you don't have a mesh"))
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

Problem &
Executioner::problem()
{
  mooseDoOnce(mooseWarning("This method is deprecated, use feProblem() instead"));
  return _fe_problem;
}

FEProblem &
Executioner::feProblem()
{
  return _fe_problem;
}

std::string
Executioner::getTimeStepperName()
{
  return std::string();
}

bool
Executioner::lastSolveConverged()
{
  return _fe_problem.converged();
}

void
Executioner::addAttributeReporter(const std::string & name, Real & attribute, const std::string execute_on)
{
  FEProblem * problem = parameters().getCheckedPointerParam<FEProblem *>("_fe_problem", "Failed to retrieve FEProblem when adding a attribute reporter in Executioner");
  InputParameters params = _app.getFactory().getValidParams("ExecutionerAttributeReporter");
  params.set<Real *>("value") = &attribute;
  if (!execute_on.empty())
    params.set<MultiMooseEnum>("execute_on") = execute_on;
  problem->addPostprocessor("ExecutionerAttributeReporter", name, params);
}
