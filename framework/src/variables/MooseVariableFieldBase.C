//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableFieldBase.h"

defineLegacyParams(MooseVariableFieldBase);

InputParameters
MooseVariableFieldBase::validParams()
{
  return MooseVariableBase::validParams();
}

MooseVariableFieldBase::MooseVariableFieldBase(const InputParameters & parameters)
  : MooseVariableBase(parameters)
{
}

bool
MooseVariableFieldBase::hasDirichletBC(const FaceInfo & /*fi*/) const
{
  mooseError("This variable type does not implement special dirichlet BC handling");
}

std::pair<bool, std::vector<const FVFluxBC *>>
MooseVariableFieldBase::getFluxBCs(const FaceInfo & /*fi*/) const
{
  mooseError("This variable type does not implement special flux BC handling");
}
