//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TwoMaterialPropertyInterface.h"
#include "MaterialData.h"
#include "InputParameters.h"

template <>
InputParameters
validParams<TwoMaterialPropertyInterface>()
{
  // Objects inheriting from TwoMaterialPropertyInterface rely on Boundary MaterialData
  InputParameters params = validParams<MaterialPropertyInterface>();
  params.set<Moose::MaterialDataType>("_material_data_type") = Moose::BOUNDARY_MATERIAL_DATA;
  return params;
}

TwoMaterialPropertyInterface::TwoMaterialPropertyInterface(const MooseObject * moose_object)
  : MaterialPropertyInterface(moose_object),
    _neighbor_material_data(_mi_feproblem.getMaterialData(Moose::NEIGHBOR_MATERIAL_DATA,
                                                          _mi_params.get<THREAD_ID>("_tid")))
{
}

TwoMaterialPropertyInterface::TwoMaterialPropertyInterface(const MooseObject * moose_object,
                                                           const std::set<SubdomainID> & blocks_ids)
  : MaterialPropertyInterface(moose_object, blocks_ids),
    _neighbor_material_data(_mi_feproblem.getMaterialData(Moose::NEIGHBOR_MATERIAL_DATA,
                                                          _mi_params.get<THREAD_ID>("_tid")))
{
}
TwoMaterialPropertyInterface::TwoMaterialPropertyInterface(
    const MooseObject * moose_object, const std::set<BoundaryID> & boundary_ids)
  : MaterialPropertyInterface(moose_object, boundary_ids),
    _neighbor_material_data(_mi_feproblem.getMaterialData(Moose::NEIGHBOR_MATERIAL_DATA,
                                                          _mi_params.get<THREAD_ID>("_tid")))
{
}

TwoMaterialPropertyInterface::TwoMaterialPropertyInterface(
    const MooseObject * moose_object,
    const std::set<SubdomainID> & blocks_ids,
    const std::set<BoundaryID> & boundary_ids)
  : MaterialPropertyInterface(moose_object, blocks_ids, boundary_ids),
    _neighbor_material_data(_mi_feproblem.getMaterialData(Moose::NEIGHBOR_MATERIAL_DATA,
                                                          _mi_params.get<THREAD_ID>("_tid")))
{
}
