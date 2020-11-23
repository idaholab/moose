//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVMassAdvection.h"

#ifdef MOOSE_GLOBAL_AD_INDEXING

registerMooseObject("NavierStokesApp", NSFVMassAdvection);

InputParameters
NSFVMassAdvection::validParams()
{
  InputParameters params = NSFVAdvectionKernel::validParams();
  params.set<MaterialPropertyName>("advected_quantity") = "rho";
  params.suppressParameter<MaterialPropertyName>("advected_quantity");
  return params;
}

NSFVMassAdvection::NSFVMassAdvection(const InputParameters & params) : NSFVAdvectionKernel(params)
{
}

#endif
