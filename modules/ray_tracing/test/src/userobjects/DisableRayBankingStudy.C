//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DisableRayBankingStudy.h"

registerMooseObject("RayTracingTestApp", DisableRayBankingStudy);

InputParameters
DisableRayBankingStudy::validParams()
{
  auto params = RepeatableRayStudy::validParams();

  params.set<bool>("_bank_rays_on_completion") = false;

  return params;
}

DisableRayBankingStudy::DisableRayBankingStudy(const InputParameters & parameters)
  : RepeatableRayStudy(parameters)
{
}
