//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshDisplacedInterface.h"

#include "FEProblem.h"

InputParameters
MeshDisplacedInterface::validParams()
{
  InputParameters params = emptyInputParameters();
  return params;
}

MeshDisplacedInterface::MeshDisplacedInterface(const InputParameters & params)
  : _mdi_feproblem(*params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
{
  _mdi_feproblem.notifyWhenMeshDisplaces(this);
}
