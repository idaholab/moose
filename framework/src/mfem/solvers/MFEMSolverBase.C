//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMSolverBase.h"

InputParameters
MFEMSolverBase::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.addClassDescription("Base class for defining mfem::Solver derived classes for Moose.");
  params.registerBase("MFEMSolverBase");
  params.addParam<bool>("low_order_refined", false, "Set usage of Low-Order Refined solver.");

  return params;
}

MFEMSolverBase::MFEMSolverBase(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters), _lor{getParam<bool>("low_order_refined")}
{
}

#endif
