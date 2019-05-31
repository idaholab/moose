//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ResetMaterialCache.h"

registerMooseObject("MooseApp", ResetMaterialCache);

template <>
InputParameters
validParams<ResetMaterialCache>()
{
  InputParameters params = validParams<GeneralUserObject>();
  return params;
}

void
ResetMaterialCache::initialize()
{
  _fe_problem.enableMaterialCache();
}

void
ResetMaterialCache::execute()
{
  _fe_problem.invalidateMaterialCache();
}
