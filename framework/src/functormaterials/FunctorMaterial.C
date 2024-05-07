//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorMaterial.h"

InputParameters
FunctorMaterial::validParams()
{
  auto params = Material::validParams();
  params.suppressParameter<bool>("use_displaced_mesh");

  // Functor materials define functors, and the functor caching is defined by the execute_on
  params += SetupInterface::validParams();
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_ALWAYS);
  // Default is no-caching. Same default as not passing a clearance schedule when adding a functor
  params.set<ExecFlagEnum>("execute_on") = {EXEC_ALWAYS};

  // Do not allow functor materials in the regular Materials block
  params.registerBase("FunctorMaterial");

  // Remove MaterialBase parameters that are not used
  params.suppressParameter<bool>("compute");

  return params;
}

FunctorMaterial::FunctorMaterial(const InputParameters & parameters) : Material(parameters) {}
