//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NumNodes.h"
#include "SubProblem.h"
#include "MooseMesh.h"

registerMooseObject("MooseApp", NumNodes);

InputParameters
NumNodes::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addClassDescription(
      "Returns the total number of nodes in a simulation (works with DistributedMesh)");
  return params;
}

NumNodes::NumNodes(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _mesh(_subproblem.mesh().getMesh())
{
}

Real
NumNodes::getValue()
{
  return _mesh.n_nodes();
}
