//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ResolveOptionalMaterialPropertiesAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp",
                    ResolveOptionalMaterialPropertiesAction,
                    "resolve_optional_materials");

InputParameters
ResolveOptionalMaterialPropertiesAction::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

ResolveOptionalMaterialPropertiesAction::ResolveOptionalMaterialPropertiesAction(
    InputParameters params)
  : Action(params)
{
}

void
ResolveOptionalMaterialPropertiesAction::act()
{
  mooseAssert(_problem, "Problem doesn't exist");

  const Moose::MaterialDataType mdt_list[] = {Moose::BLOCK_MATERIAL_DATA,
                                              Moose::FACE_MATERIAL_DATA,
                                              Moose::NEIGHBOR_MATERIAL_DATA,
                                              Moose::INTERFACE_MATERIAL_DATA};

  for (auto mdt : mdt_list)
  {
    auto & materials = _problem->getMaterialWarehouse()[mdt];

    for (auto tid : make_range(materials.numThreads()))
      for (auto matbase_ptr : materials.getObjects(tid))
        if (auto mat_ptr = std::dynamic_pointer_cast<Material>(matbase_ptr))
          mat_ptr->resolveOptionalProperties();
  }
}
