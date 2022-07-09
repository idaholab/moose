//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThreeMaterialPropertyInterface.h"
#include "MaterialData.h"
#include "InputParameters.h"
#include "FEProblemBase.h"

InputParameters
ThreeMaterialPropertyInterface::validParams()
{

  // Objects inheriting from ThreeMaterialPropertyInterface rely on Boundary MaterialData
  InputParameters params = TwoMaterialPropertyInterface::validParams();
  // We want the properties returned by getMaterialProperty* to be the volumetric ones
  params.set<Moose::MaterialDataType>("_material_data_type") = Moose::BLOCK_MATERIAL_DATA;
  return params;
}

ThreeMaterialPropertyInterface::ThreeMaterialPropertyInterface(
    const MooseObject * moose_object,
    const std::set<SubdomainID> & blocks_ids,
    const std::set<BoundaryID> & boundary_ids)
  : TwoMaterialPropertyInterface(moose_object, blocks_ids, boundary_ids),
    _face_material_data(
        _mi_feproblem.getMaterialData(Moose::FACE_MATERIAL_DATA, _mi_params.get<THREAD_ID>("_tid")))
{
}
