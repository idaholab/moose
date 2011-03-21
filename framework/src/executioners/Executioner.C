#include "Executioner.h"

// Moose includes
#include "Moose.h"
#include "SubProblem.h"

// C++ includes
#include <vector>
#include <limits>


template<>
InputParameters validParams<Executioner>()
{
  InputParameters params = validParams<Object>();
  params.addPrivateParam<Moose::SubProblem *>("_subproblem");
  return params;
}

Executioner::Executioner(const std::string & name, InputParameters parameters) :
  Object(name, parameters),
  _problem(*parameters.get<Moose::SubProblem *>("_subproblem")),
  _output_initial(false)
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

