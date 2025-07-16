//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMComplexIntegratedBC.h"

InputParameters
MFEMComplexIntegratedBC::validParams()
{
  InputParameters params = MFEMBoundaryCondition::validParams();
  params.addParam<UserObjectName>(
      "real_bc", "Name of the integrated BC to use for the real of the composite complex BC.");
  params.addParam<UserObjectName>(
      "imag_bc", "Name of the integrated BC to use for the imaginary of the composite complex BC.");

  return params;
}

MFEMComplexIntegratedBC::MFEMComplexIntegratedBC(const InputParameters & parameters)
  : MFEMBoundaryCondition(parameters)
{
  //FEProblemBase::addUserObject(kernel_name, name, parameters);
  //const UserObject * kernel_uo = &(getUserObjectBase(name));

  _real_bc = std::dynamic_pointer_cast<MFEMIntegratedBC>(getUserObject<MFEMIntegratedBC>("real_kernel").getSharedPtr());
  _imag_bc = std::dynamic_pointer_cast<MFEMIntegratedBC>(getUserObject<MFEMIntegratedBC>("imag_kernel").getSharedPtr());

}

#endif
