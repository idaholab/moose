//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshChangedInterface.h"

#include "FEProblem.h"

InputParameters
MeshChangedInterface::validParams()
{

  InputParameters params = emptyInputParameters();
  return params;
}

MeshChangedInterface::MeshChangedInterface(const InputParameters & params)
  : _mci_feproblem(*params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
{
  _mci_feproblem.notifyWhenMeshChanges(this);
}
