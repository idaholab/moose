//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMWeakFormBase.h"
#include "TimeDependentEquationSystem.h"
#include "EigenproblemEquationSystem.h"
#include "ComplexEquationSystem.h"
#include "MFEMEigenproblem.h"

InputParameters
MFEMWeakFormBase::validParams()
{
  InputParameters params = MFEMObject::validParams();
  params.registerBase("MFEMWeakFormBase");
  params.registerSystemAttributeName("MFEMWeakFormBase");
  params.addParam<std::vector<MFEMBoundaryConditionName>>(
      "bcs", {}, "List of boundary conditions to add to the weak form");
  params.addParam<std::vector<MFEMKernelName>>(
      "kernels", {}, "List of kernels to add to the weak form");
  return params;
}

MFEMWeakFormBase::MFEMWeakFormBase(const InputParameters & parameters)
  : MFEMObject(parameters),
    _bc_names(getParam<std::vector<MFEMBoundaryConditionName>>("bcs")),
    _kernel_names(getParam<std::vector<MFEMKernelName>>("kernels"))
{
}

#endif
