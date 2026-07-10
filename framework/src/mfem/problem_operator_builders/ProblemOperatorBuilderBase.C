//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "ProblemOperatorBuilderBase.h"
#include "MFEMProblem.h"
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/mesh_extras.hpp"
#include "libmesh/restore_warnings.h"

InputParameters
Moose::MFEM::ProblemOperatorBuilderBase::validParams()
{
  InputParameters params = MFEMObject::validParams();
  params.registerBase("Moose::MFEM::ProblemOperatorBuilderBase");
  params.registerSystemAttributeName("Moose::MFEM::ProblemOperatorBuilderBase");
  params.addParam<VariableName>("variable", "Variable labelling the problem operator");
  return params;
}

Moose::MFEM::ProblemOperatorBuilderBase::ProblemOperatorBuilderBase(
    const InputParameters & parameters)
  : MFEMObject(parameters)
{
}

#endif
