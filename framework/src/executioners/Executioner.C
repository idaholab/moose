#include "Executioner.h"

// Moose includes
#include "Moose.h"
#include "Mesh.h"

// C++ includes
#include <vector>
#include <limits>


template<>
InputParameters validParams<Executioner>()
{
  InputParameters params = validParams<Object>();
  return params;
}

Executioner::Executioner(const std::string & name, InputParameters parameters) :
  Object(name, parameters),
  _mesh(getParam<Moose::Mesh *>("_mesh")),
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

