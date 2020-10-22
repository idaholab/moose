//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVFluxBCBase.h"
#include "MooseVariableFV.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "ADUtils.h"

InputParameters
FVFluxBCBase::validParams()
{
  InputParameters params = FVBoundaryCondition::validParams();
  params += MaterialPropertyInterface::validParams();
  params.registerSystemAttributeName("FVFluxBC");

  // FVFluxBCBases always rely on Boundary MaterialData
  params.set<Moose::MaterialDataType>("_material_data_type") = Moose::BOUNDARY_MATERIAL_DATA;

  return params;
}

FVFluxBCBase::FVFluxBCBase(const InputParameters & parameters)
  : FVBoundaryCondition(parameters),
    CoupleableMooseVariableDependencyIntermediateInterface(this, /*nodal=*/false, /*is_fv=*/true),
    MaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, boundaryIDs())
{
}
