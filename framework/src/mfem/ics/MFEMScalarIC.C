//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMScalarIC.h"
#include "MFEMProblem.h"
#include <mfem.hpp>

registerMooseObject("MooseApp", MFEMScalarIC);

InputParameters
MFEMScalarIC::validParams()
{
  auto params = MFEMInitialCondition::validParams();
  params.addClassDescription("Sets the initial values of an MFEM scalar variable from a "
                             "user-specified scalar coefficient.");
  params.addRequiredParam<MFEMScalarCoefficientName>("coefficient", "The scalar coefficient");
  return params;
}

MFEMScalarIC::MFEMScalarIC(const InputParameters & params) : MFEMInitialCondition(params) {}

void
MFEMScalarIC::execute()
{
  auto & coeff = getScalarCoefficient("coefficient");
  auto grid_function = getMFEMProblem().getGridFunction(getParam<VariableName>("variable"));
  grid_function->ProjectCoefficient(coeff);
}

#endif
