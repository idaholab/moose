//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Base class to move elements to a specific subdomain at the given times.

#include "TimedElementSubdomainModifier.h"

InputParameters
TimedElementSubdomainModifier::validParams()
{
  InputParameters params = ElementSubdomainModifier::validParams();
  params.addParam<std::vector<Real>>("times", "The times of the subdomain modifications.");
  return params;
}

TimedElementSubdomainModifier::TimedElementSubdomainModifier(const InputParameters & parameters)
  : ElementSubdomainModifier(parameters)
{
}

void
TimedElementSubdomainModifier::initialize()
{
  // ask for all times (must NOT be sorted)
  const auto _times = onGetTimes();
  const auto n = _times.size();

  // copy data to local storage
  _times_and_indices.resize(n);
  for (auto i : make_range(n))
  {
    _times_and_indices[i].time = _times[i];
    _times_and_indices[i].index = i;
  };

  // sort the times
  std::sort(_times_and_indices.begin(), _times_and_indices.end());
}
