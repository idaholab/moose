//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ResolveOptionalMaterialPropertiesAction.h"
#include "MaterialPropertyInterface.h"
#include "MooseApp.h"

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
    const InputParameters & params)
  : Action(params)
{
}

void
ResolveOptionalMaterialPropertiesAction::act()
{
  mooseAssert(_problem, "Problem doesn't exist");

  for (auto mpi : _app.getInterfaceObjects<MaterialPropertyInterface>())
    mpi->resolveOptionalProperties();
}
