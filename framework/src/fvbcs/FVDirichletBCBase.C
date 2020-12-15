//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVDirichletBCBase.h"

InputParameters
FVDirichletBCBase::validParams()
{
  InputParameters params = FVBoundaryCondition::validParams();
  params.addClassDescription("Defines a Dirichlet boundary condition for finite volume method.");
  params.registerSystemAttributeName("FVDirichletBC");
  return params;
}

FVDirichletBCBase::FVDirichletBCBase(const InputParameters & parameters)
  : FVBoundaryCondition(parameters)
{
}
