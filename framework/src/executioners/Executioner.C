#include "Executioner.h"

// Moose includes
#include "Moose.h"
#include "Problem.h"

// C++ includes
#include <vector>
#include <limits>


template<>
InputParameters validParams<Executioner>()
{
  InputParameters params = validParams<Object>();
  params.addPrivateParam<Moose::Problem *>("_problem");
  return params;
}

Executioner::Executioner(const std::string & name, InputParameters parameters) :
  Object(name, parameters),
  _problem(*parameters.get<Moose::Problem *>("_problem")),
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

