//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexPointVariableValueSampler.h"

registerMooseObject("MooseApp", MFEMComplexPointVariableValueSampler);

InputParameters
MFEMComplexPointVariableValueSampler::validParams()
{
  InputParameters params = MFEMComplexVariableValueSamplerBase::validParams();
  params.addClassDescription("Sample a complex MFEM variable at specific points, outputting "
                             "real and imaginary parts as separate columns.");
  params.addRequiredParam<std::vector<Point>>("points",
                                              "The points where you want to evaluate the variable");
  return params;
}

MFEMComplexPointVariableValueSampler::MFEMComplexPointVariableValueSampler(
    const InputParameters & parameters)
  : MFEMComplexVariableValueSamplerBase(parameters, parameters.get<std::vector<Point>>("points"))
{
}

#endif // MOOSE_MFEM_ENABLED
