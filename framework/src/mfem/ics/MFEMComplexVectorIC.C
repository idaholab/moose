//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexVectorIC.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMComplexVectorIC);

InputParameters
MFEMComplexVectorIC::validParams()
{
  auto params = MFEMInitialCondition::validParams();
  params.addClassDescription("Sets the initial values of an MFEM vector variable from a "
                             "user-specified vector coefficient.");
  params.addRequiredParam<MFEMVectorCoefficientName>("vector_coefficient_real",
                                                     "The real part of the vector coefficient");
  params.addRequiredParam<MFEMVectorCoefficientName>("vector_coefficient_imag",
                                                     "The imaginary part of the vector coefficient");
  return params;
}

MFEMComplexVectorIC::MFEMComplexVectorIC(const InputParameters & params) : MFEMInitialCondition(params) {}

void
MFEMComplexVectorIC::execute()
{
  auto & coeff_real = getVectorCoefficient("vector_coefficient_real");
  auto & coeff_imag = getVectorCoefficient("vector_coefficient_imag");
  auto grid_function = getMFEMProblem().getComplexGridFunction(getParam<VariableName>("variable"));
  grid_function->ProjectCoefficient(coeff_real, coeff_imag);
}

#endif
