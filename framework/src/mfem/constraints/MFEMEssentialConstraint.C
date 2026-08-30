//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMEssentialConstraint.h"

InputParameters
MFEMEssentialConstraint::validParams()
{
  InputParameters params = MFEMConstraint::validParams();
  params.addClassDescription(
      "Base class for applying essential volumetric constraints to MFEM problems.");
  return params;
}

MFEMEssentialConstraint::MFEMEssentialConstraint(const InputParameters & parameters)
  : MFEMConstraint(parameters)
{
}

#endif
