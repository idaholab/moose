//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestGhostBoundarySideUserObject.h"
#include "MooseMesh.h"

registerMooseObject("MooseTestApp", TestGhostBoundarySideUserObject);

InputParameters
TestGhostBoundarySideUserObject::validParams()
{
  InputParameters params = SideUserObject::validParams();
  params.addRelationshipManager("GhostBoundary",
                                Moose::RelationshipManagerType::GEOMETRIC,
                                [](const InputParameters & obj_params, InputParameters & rm_params)
                                {
                                  rm_params.set<std::vector<BoundaryName>>("boundary") =
                                      obj_params.get<std::vector<BoundaryName>>("boundary");
                                });
  return params;
}

TestGhostBoundarySideUserObject::TestGhostBoundarySideUserObject(const InputParameters & parameters)
  : SideUserObject(parameters)
{
}
