//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMPointVariableValueSampler.h"

registerMooseObject("MooseApp", MFEMPointVariableValueSampler);
registerMooseObjectRenamed("MooseApp",
                           MFEMPointValueSampler,
                           "06/30/2027 24:00",
                           MFEMPointVariableValueSampler);

InputParameters
MFEMPointVariableValueSampler::validParams()
{
  InputParameters params = MFEMVariableValueSamplerBase::validParams();

  params.addClassDescription("Sample an MFEM variable at specific points.");
  params.addRequiredParam<std::vector<Point>>(
      "points", "The points where you want to evaluate the variables");

  return params;
}

MFEMPointVariableValueSampler::MFEMPointVariableValueSampler(const InputParameters & parameters)
  : MFEMVariableValueSamplerBase(parameters, parameters.get<std::vector<Point>>("points"))
{
}

#endif // MOOSE_MFEM_ENABLED
