//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMVectorIC.h"
#include "MFEMProblem.h"
#include <libmesh/libmesh_common.h>
#include <mfem.hpp>

registerMooseObject("MooseApp", MFEMVectorIC);

InputParameters
MFEMVectorIC::validParams()
{
  auto params = MFEMInitialCondition::validParams();
  params.addClassDescription("Sets the initial values of an MFEM vector variable from a "
                             "user-specified vector coefficient.");
  params.addRequiredParam<MFEMVectorCoefficientName>("coefficient", "The vector coefficient");
  return params;
}

MFEMVectorIC::MFEMVectorIC(const InputParameters & params) : MFEMInitialCondition(params) {}

void
MFEMVectorIC::execute()
{
  auto & coeff = getVectorCoefficient(getParam<MFEMVectorCoefficientName>("coefficient"));
  auto grid_function = getMFEMProblem().getGridFunction(getParam<VariableName>("variable"));
  grid_function->ProjectCoefficient(coeff);
}

#endif
