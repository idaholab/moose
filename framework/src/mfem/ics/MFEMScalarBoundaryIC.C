//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMScalarBoundaryIC.h"
#include "MFEMProblem.h"
#include <mfem.hpp>

registerMooseObject("MooseApp", MFEMScalarBoundaryIC);

InputParameters
MFEMScalarBoundaryIC::validParams()
{
  auto params = MFEMInitialCondition::validParams();
  params += MFEMBoundaryRestrictable::validParams();
  params.addClassDescription("Sets the initial values of an MFEM scalar variable from a "
                             "user-specified scalar coefficient.");
  params.addRequiredParam<MFEMScalarCoefficientName>("coefficient", "The scalar coefficient");
  return params;
}

MFEMScalarBoundaryIC::MFEMScalarBoundaryIC(const InputParameters & params)
  : MFEMInitialCondition(params),
    MFEMBoundaryRestrictable(params,
                             *getMFEMProblem()
                                  .getProblemData()
                                  .gridfunctions.GetRef(getParam<VariableName>("variable"))
                                  .ParFESpace()
                                  ->GetParMesh())
{
}

void
MFEMScalarBoundaryIC::execute()
{
  auto & coeff = getScalarCoefficient("coefficient");
  auto grid_function = getMFEMProblem().getGridFunction(getParam<VariableName>("variable"));
  grid_function->ProjectBdrCoefficient(coeff, getBoundaryMarkers());
}

#endif
