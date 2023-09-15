//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PWCNSFVMassAdvection.h"
#include "NS.h"
#include "NSFVUtils.h"

registerMooseObject("NavierStokesApp", PWCNSFVMassAdvection);

InputParameters
PWCNSFVMassAdvection::validParams()
{
  auto params = PINSFVMassAdvection::validParams();
  params.addClassDescription("Object for advecting mass in porous media mass equation");
  return params;
}

PWCNSFVMassAdvection::PWCNSFVMassAdvection(const InputParameters & params)
  : PINSFVMassAdvection(params)
{
}
