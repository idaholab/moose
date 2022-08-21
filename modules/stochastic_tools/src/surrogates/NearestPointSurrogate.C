//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NearestPointSurrogate.h"

registerMooseObject("StochasticToolsApp", NearestPointSurrogate);

InputParameters
NearestPointSurrogate::validParams()
{
  InputParameters params = SurrogateModel::validParams();
  params.addClassDescription("Surrogate that evaluates the value from the nearest point from data "
                             "in [NearestPointTrainer.md]");
  return params;
}

NearestPointSurrogate::NearestPointSurrogate(const InputParameters & parameters)
  : SurrogateModel(parameters),
    _sample_points(getModelData<std::vector<std::vector<Real>>>("_sample_points")),
    _sample_results(getModelData<std::vector<std::vector<Real>>>("_sample_results"))
{
}

Real
NearestPointSurrogate::evaluate(const std::vector<Real> & x) const
{
  // Check whether input point has same dimensionality as training data
  mooseAssert(_sample_points.size() == x.size(),
              "Input point does not match dimensionality of training data.");

  return _sample_results[0][findNearestPoint(x)];
}

void
NearestPointSurrogate::evaluate(const std::vector<Real> & x, std::vector<Real> & y) const
{
  mooseAssert(_sample_points.size() == x.size(),
              "Input point does not match dimensionality of training data.");

  y.assign(_sample_results.size(), 0.0);

  unsigned int idx = findNearestPoint(x);

  for (const auto & r : index_range(y))
    y[r] = _sample_results[r][idx];
}

unsigned int
NearestPointSurrogate::findNearestPoint(const std::vector<Real> & x) const
{
  unsigned int idx = 0;

  // Container of current minimum distance during training sample loop
  Real dist_min = std::numeric_limits<Real>::max();

  for (dof_id_type p = 0; p < _sample_points[0].size(); ++p)
  {
    // Sum over the distance of each point dimension
    Real dist = 0;
    for (unsigned int i = 0; i < x.size(); ++i)
    {
      Real diff = (x[i] - _sample_points[i][p]);
      dist += diff * diff;
    }

    // Check if this training point distance is smaller than the current minimum
    if (dist < dist_min)
    {
      idx = p;
      dist_min = dist;
    }
  }
  return idx;
}
