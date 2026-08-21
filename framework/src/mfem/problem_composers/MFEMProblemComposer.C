//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMProblemComposer.h"
#include "MFEMProblem.h"
#include "ProblemOperatorBase.h"
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/mesh_extras.hpp"
#include "libmesh/restore_warnings.h"

InputParameters
Moose::MFEM::MFEMProblemComposer::validParams()
{
  InputParameters params = MFEMObject::validParams();
  params.registerBase("Moose::MFEM::MFEMProblemComposer");
  params.registerSystemAttributeName("Moose::MFEM::MFEMProblemComposer");
  return params;
}

Moose::MFEM::MFEMProblemComposer::MFEMProblemComposer(const InputParameters & parameters)
  : MFEMObject(parameters)
{
}

#endif
