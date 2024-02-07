//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CheckActiveMatPropProblem.h"
#include "ActiveGenericConstantMaterial.h"

registerMooseObject("MooseTestApp", CheckActiveMatPropProblem);

InputParameters
CheckActiveMatPropProblem::validParams()
{
  InputParameters params = FEProblem::validParams();
  return params;
}

CheckActiveMatPropProblem::CheckActiveMatPropProblem(const InputParameters & params)
  : FEProblem(params)
{
}

std::unordered_set<unsigned int>
CheckActiveMatPropProblem::getActiveMaterialProperties(const THREAD_ID tid) const
{
  std::unordered_set<unsigned int> ret;

  // get active properties for materials
  for (auto & mat : _all_materials.getObjects(tid))
  {
    auto & check_mat = static_cast<ActiveGenericConstantMaterial &>(*mat);
    const auto & active_props = check_mat.getActivePropIDs();
    ret.insert(active_props.begin(), active_props.end());
  }
  for (auto & mat : _all_materials[Moose::FACE_MATERIAL_DATA].getObjects(tid))
  {
    auto & check_mat = static_cast<ActiveGenericConstantMaterial &>(*mat);
    const auto & active_props = check_mat.getActivePropIDs();
    ret.insert(active_props.begin(), active_props.end());
  }
  for (auto & mat : _all_materials[Moose::NEIGHBOR_MATERIAL_DATA].getObjects(tid))
  {
    auto & check_mat = static_cast<ActiveGenericConstantMaterial &>(*mat);
    const auto & active_props = check_mat.getActivePropIDs();
    ret.insert(active_props.begin(), active_props.end());
  }

  return ret;
}
