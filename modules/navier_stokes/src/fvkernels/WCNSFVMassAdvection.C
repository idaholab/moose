//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVMassAdvection.h"

registerMooseObject("NavierStokesApp", WCNSFVMassAdvection);

InputParameters
WCNSFVMassAdvection::validParams()
{
  auto params = INSFVMassAdvection::validParams();
  params.addClassDescription("Object for advecting mass, e.g. rho");
  return params;
}

WCNSFVMassAdvection::WCNSFVMassAdvection(const InputParameters & params)
  : INSFVMassAdvection(params)
{
}
