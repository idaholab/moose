//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TwoRMTester.h"
#include "MooseMesh.h"

// invalid_processor_id
#include "libmesh/dof_object.h"

registerMooseObject("MooseTestApp", TwoRMTester);

InputParameters
TwoRMTester::validParams()
{
  InputParameters params = ElemSideNeighborLayersTester::validParams();

  // Our base class had called out some relationship managers that we don't want for this object
  params.clearRelationshipManagers();

  params.addRelationshipManager(
      "ElementSideNeighborLayers",
      Moose::RelationshipManagerType::GEOMETRIC,

      [](const InputParameters & /*obj_params*/, InputParameters & rm_params)
      { rm_params.set<unsigned short>("layers") = 2; }

  );

  params.addRelationshipManager(
      "ElementSideNeighborLayers",
      Moose::RelationshipManagerType::ALGEBRAIC,

      [](const InputParameters & /*obj_params*/, InputParameters & rm_params)
      { rm_params.set<unsigned short>("layers") = 1; }

  );

  params.addClassDescription(
      "Tests that the same RM can be used with different numbers of layers for the same object.");

  return params;
}

TwoRMTester::TwoRMTester(const InputParameters & parameters)
  : ElemSideNeighborLayersTester(parameters)
{
}
