//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMPointValueSampler.h"

registerMooseMFEMObject("MooseApp", PointValueSampler);

namespace Moose::MFEM
{
InputParameters
PointValueSampler::validParams()
{
  InputParameters params = ValueSamplerBase::validParams();

  params.addClassDescription("Sample an MFEM variable at specific points.");
  params.addRequiredParam<std::vector<Point>>(
      "points", "The points where you want to evaluate the variables");

  return params;
}

PointValueSampler::PointValueSampler(const InputParameters & parameters)
  : ValueSamplerBase(parameters, parameters.get<std::vector<Point>>("points"))
{
}

} // namespace Moose::MFEM
#endif // MOOSE_MFEM_ENABLED
