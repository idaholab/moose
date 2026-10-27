//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVariableLineValueSampler.h"

#include "libmesh/point.h"
#include "MooseError.h"
#include "MFEMProblem.h"

#include <vector>

registerMooseObject("MooseApp", MFEMVariableLineValueSampler);

InputParameters
MFEMVariableLineValueSampler::validParams()
{
  InputParameters params = MFEMVariableValueSamplerBase::validParams();

  params.addClassDescription("Sample an MFEM variable along a specified line.");

  // these should not be of type libmesh::Point - need mfem::Point parsing
  params.addRequiredParam<Point>("start_point", "The beginning of the line");
  params.addRequiredParam<Point>("end_point", "The ending of the line");

  params.addRequiredRangeCheckedParam<unsigned int>(
      "num_points", "num_points >= 2", "The number of points to sample along the line");

  return params;
}

MFEMVariableLineValueSampler::MFEMVariableLineValueSampler(const InputParameters & parameters)
  : MFEMVariableValueSamplerBase(
        parameters,
        // can't call getParam as that requires initialized base class
        // so calling parameters.get directly
        Moose::MFEM::generateLinePoints(parameters.get<Point>("start_point"),
                                        parameters.get<Point>("end_point"),
                                        parameters.get<unsigned int>("num_points")))
{
}

#endif // MOOSE_MFEM_ENABLED
