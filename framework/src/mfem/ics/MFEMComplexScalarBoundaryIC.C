//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexScalarBoundaryIC.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMComplexScalarBoundaryIC);

InputParameters
MFEMComplexScalarBoundaryIC::validParams()
{
  auto params = MFEMInitialCondition::validParams();
  params += MFEMBoundaryRestrictable::validParams();
  params.addClassDescription("Sets the initial values of a complex MFEM scalar variable from a "
                             "pair of user-specified scalar coefficients.");
  params.addRequiredParam<MFEMScalarCoefficientName>("coefficient_real", "The real part of the scalar coefficient");
  params.addRequiredParam<MFEMScalarCoefficientName>("coefficient_imag", "The imaginary part of the scalar coefficient");
  return params;
}

MFEMComplexScalarBoundaryIC::MFEMComplexScalarBoundaryIC(const InputParameters & params)
  : MFEMInitialCondition(params),
    MFEMBoundaryRestrictable(params,
                             *getMFEMProblem()
                                  .getProblemData()
                                  .cmplx_gridfunctions.GetRef(getParam<VariableName>("variable"))
                                  .ParFESpace()
                                  ->GetParMesh())
{
}

void
MFEMComplexScalarBoundaryIC::execute()
{
  auto & coeff_real = getScalarCoefficient("coefficient_real");
  auto & coeff_imag = getScalarCoefficient("coefficient_imag");
  auto grid_function = getMFEMProblem().getComplexGridFunction(getParam<VariableName>("variable"));
  grid_function->ProjectBdrCoefficient(coeff_real, coeff_imag, getBoundaryMarkers());
}

#endif
