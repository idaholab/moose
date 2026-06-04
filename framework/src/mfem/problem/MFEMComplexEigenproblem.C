//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexEigenproblem.h"

registerMooseObject("MooseApp", MFEMComplexEigenproblem);

InputParameters
MFEMComplexEigenproblem::validParams()
{
  InputParameters params = MFEMEigenproblemBase::validParams();
  params.addClassDescription("Problem type for building and solving a complex-valued finite element "
                             "eigenproblem using the MFEM finite element library.");
  return params;
}

MFEMComplexEigenproblem::MFEMComplexEigenproblem(const InputParameters & params)
  : MFEMEigenproblemBase(params)
{
  if (getNumericType() != NumericType::COMPLEX)
    mooseError("MFEMComplexEigenproblem requires 'numeric_type = complex' in the [Problem] block.");
}

#endif
