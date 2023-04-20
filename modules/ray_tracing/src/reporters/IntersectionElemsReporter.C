//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IntersectionElemsReporter.h"
#include "RayTracingOverlayMeshMapping.h"

registerMooseObject("RayTracingApp", IntersectionElemsReporter);

InputParameters
IntersectionElemsReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addRequiredParam<UserObjectName>(
      "overlay_uo_name",
      "The name of the RayTracingOverlayMeshMapping user object to pull data from");
  params.addParam<bool>("serialize", false, "True to serialize the data on rank 0");
  params.addClassDescription("Reports the elems mapping between ovelay mesh and main mesh.");
  return params;
}

IntersectionElemsReporter::IntersectionElemsReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _serialize(getParam<bool>("serialize")),
    _overlay_mesh_mapping(getUserObject<RayTracingOverlayMeshMapping>("overlay_uo_name")),
    _to_overlay(declareValueByName<std::map<dof_id_type, std::set<dof_id_type>>>(
        "to_overlay", _serialize ? REPORTER_MODE_ROOT : REPORTER_MODE_DISTRIBUTED)),
    _from_overlay(declareValueByName<std::map<dof_id_type, std::set<dof_id_type>>>(
        "from_overlay", _serialize ? REPORTER_MODE_ROOT : REPORTER_MODE_DISTRIBUTED))
{
}

void
IntersectionElemsReporter::execute()
{
  _to_overlay = _overlay_mesh_mapping.overlayIDMap(true, _serialize);
  _from_overlay = _overlay_mesh_mapping.overlayIDMap(false, _serialize);
}
