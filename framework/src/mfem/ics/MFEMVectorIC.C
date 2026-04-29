//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorIC.h"
#include "MFEMProblem.h"

registerMooseMFEMObject("MooseApp", VectorIC);

namespace Moose::MFEM
{
InputParameters
VectorIC::validParams()
{
  auto params = InitialCondition::validParams();
  params.addClassDescription("Sets the initial values of an MFEM vector variable from a "
                             "user-specified vector coefficient.");
  params.addRequiredParam<Moose::MFEM::VectorCoefficientName>("vector_coefficient",
                                                              "The vector coefficient");
  return params;
}

VectorIC::VectorIC(const InputParameters & params) : InitialCondition(params) {}

void
VectorIC::execute()
{
  auto & coeff = getVectorCoefficient("vector_coefficient");
  auto grid_function = getMFEMProblem().getGridFunction(getParam<VariableName>("variable"));
  grid_function->ProjectCoefficient(coeff);
}

} // namespace Moose::MFEM
#endif
