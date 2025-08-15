//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMLineValueSampler.h"

#include "libmesh/point.h"
#include "MooseError.h"
#include "MFEMProblem.h"

#include <vector>

registerMooseObject("MooseApp", MFEMLineValueSampler);

namespace
{
std::vector<Point>
generateLinePoints(const Point & start_point, const Point & end_point, unsigned int num_points)
{
  if (num_points < 2)
  {
    mooseError("In MFEMLineValueSampler: line must have at least 2 points,"
               "for single points use MFEMPointValueSampler.");
  }

  // initialize and populate vector with linearly-spaced points along line
  std::vector<Point> points;
  points.reserve(num_points);
  for (unsigned int i_point = 0; i_point < num_points; i_point++)
  {
    // fractional distance along line [0, 1]
    Real t = static_cast<Real>(i_point) / static_cast<Real>(num_points - 1);
    points.push_back(t * end_point + (1 - t) * start_point);
  }

  return points;
}
}

InputParameters
MFEMLineValueSampler::validParams()
{
  InputParameters params = MFEMValueSamplerBase::validParams();

  params.addClassDescription("Sample an MFEM variable along a specified line.");

  // these should not be of type libmesh::Point - need mfem::Point parsing
  params.addRequiredParam<Point>("start_point", "The beginning of the line");
  params.addRequiredParam<Point>("end_point", "The ending of the line");

  params.addRequiredParam<unsigned int>("num_points",
                                        "The number of points to sample along the line");

  return params;
}

MFEMLineValueSampler::MFEMLineValueSampler(const InputParameters & parameters)
  : MFEMValueSamplerBase(parameters,
                         // can't call getParam as that requires initialized base class
                         // so calling parameters.get directly
                         generateLinePoints(parameters.get<Point>("start_point"),
                                            parameters.get<Point>("end_point"),
                                            parameters.get<unsigned int>("num_points")))
{
}

#endif // MOOSE_MFEM_ENABLED
