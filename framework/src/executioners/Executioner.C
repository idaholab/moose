#include "Executioner.h"

// Moose includes
#include "Moose.h"
#include "MooseMesh.h"

// C++ includes
#include <vector>
#include <limits>


template<>
InputParameters validParams<Executioner>()
{
  InputParameters params = validParams<MooseObject>();
  return params;
}

Executioner::Executioner(const std::string & name, InputParameters parameters) :
  MooseObject(name, parameters),
  _mesh(getParam<MooseMesh *>("_mesh")),
  _output_initial(false)
{
}

Executioner::~Executioner()
{
  delete _mesh;
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

