//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUIntegratedBCBase.h"

InputParameters
GPUIntegratedBCBase::validParams()
{
  auto params = GPUBoundaryCondition::validParams();
  params += MaterialPropertyInterface::validParams();

  // Integrated BCs always rely on Boundary MaterialData
  params.set<Moose::MaterialDataType>("_material_data_type") = Moose::BOUNDARY_MATERIAL_DATA;

  return params;
}

GPUIntegratedBCBase::GPUIntegratedBCBase(const InputParameters & parameters,
                                         Moose::VarFieldType field_type)
  : GPUBoundaryCondition(parameters, field_type, false),
    CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    MaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, boundaryIDs())
{
}

GPUIntegratedBCBase::GPUIntegratedBCBase(const GPUIntegratedBCBase & object)
  : GPUBoundaryCondition(object),
    CoupleableMooseVariableDependencyIntermediateInterface(&object, false, false, false),
    MaterialPropertyInterface(&object, Moose::EMPTY_BLOCK_IDS, boundaryIDs(), false)
{
}
