/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "TwoMaterialPropertyInterface.h"
#include "MaterialData.h"
#include "InputParameters.h"
#include "FEProblem.h"

template<>
InputParameters validParams<TwoMaterialPropertyInterface>()
{
  // Objects inheriting from TwoMaterialPropertyInterface rely on Boundary MaterialData
  InputParameters params = validParams<MaterialPropertyInterface>();
  params.set<Moose::MaterialDataType>("_material_data_type") = Moose::BOUNDARY_MATERIAL_DATA;
  return params;
}

TwoMaterialPropertyInterface::TwoMaterialPropertyInterface(const MooseObject * moose_object) :
    MaterialPropertyInterface(moose_object),
    _neighbor_material_data(_mi_feproblem.getMaterialData(Moose::NEIGHBOR_MATERIAL_DATA, _mi_params.get<THREAD_ID>("_tid")))
{
}

TwoMaterialPropertyInterface::TwoMaterialPropertyInterface(const MooseObject * moose_object, const std::set<SubdomainID> & block_ids) :
    MaterialPropertyInterface(moose_object, block_ids),
    _neighbor_material_data(_mi_feproblem.getMaterialData(Moose::NEIGHBOR_MATERIAL_DATA, _mi_params.get<THREAD_ID>("_tid")))
{
}
