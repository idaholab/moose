//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseGhosting.h"
#include "InputParameters.h"
#include "MooseTypes.h"

namespace Moose
{
InputParameters
dummyParams()
{
  auto params = emptyInputParameters();
  params.set<std::string>("_moose_base") = "dummy";
  return params;
}

InputParameters
zeroLayerGhosting(RelationshipManagerType rm_type)
{
  auto params = dummyParams();
  params.addRelationshipManager("ElementSideNeighborLayers",
                                rm_type,
                                [](const InputParameters &, InputParameters & rm_params) {
                                  rm_params.set<unsigned short>("layers") = 0;
                                });
  return params;
}

InputParameters
oneLayerGhosting(RelationshipManagerType rm_type)
{
  auto params = dummyParams();
  params.addRelationshipManager("ElementSideNeighborLayers", rm_type);
  return params;
}
}
