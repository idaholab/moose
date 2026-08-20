//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "TestGhostBoundarySideUserObject.h"
#include "MooseMesh.h"

registerMooseObject("SolidMechanicsTestApp", TestSolidMechanicsGhostBoundarySideUserObject);

InputParameters
TestSolidMechanicsGhostBoundarySideUserObject::validParams()
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

TestSolidMechanicsGhostBoundarySideUserObject::TestSolidMechanicsGhostBoundarySideUserObject(
    const InputParameters & parameters)
  : SideUserObject(parameters)
{
}
