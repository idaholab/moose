//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMassAdvection.h"

#ifdef MOOSE_GLOBAL_AD_INDEXING

registerMooseObject("NavierStokesApp", INSFVMassAdvection);

InputParameters
INSFVMassAdvection::validParams()
{
  InputParameters params = INSFVMomentumAdvection::validParams();
  params.set<MaterialPropertyName>("advected_quantity") = "rho";
  params.suppressParameter<MaterialPropertyName>("advected_quantity");
  return params;
}

INSFVMassAdvection::INSFVMassAdvection(const InputParameters & params) : INSFVMomentumAdvection(params)
{
}

#endif
