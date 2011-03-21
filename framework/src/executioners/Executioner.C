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
  params.addPrivateParam<unsigned int>("steps", 0);

  params.addPrivateParam<std::string>("built_by_action", "setup_executioner");
  return params;
}

Executioner::Executioner(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    _mesh(getParam<MooseMesh *>("_mesh")),
    _output_initial(false),
    _initial_residual_norm(std::numeric_limits<Real>::max()),
    _old_initial_residual_norm(std::numeric_limits<Real>::max())
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

