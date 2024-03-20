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
#include "DelimitedFileReaderOfString.h"
#include "MooseMesh.h"

registerMooseObject("MooseApp", TimedElementSubdomainModifier);

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
  // state variables
  _last_t = -1;
  _current_step = _t_step - 1;

  // ask for all times (must NOT be sorted)
  std::vector<real> _times = onGetTimes();
  size_t n = _times.size();

  // copy data to local storage
  _timesAndIndices.resize(n);
  for (size_t i = 0; i < n; ++i)
  {
    _timesAndIndices[i].time = _times[i];
    _timesAndIndices[i].index = i;
  };

  // sort the times
  std::sort(_timesAndIndices.begin(), _timesAndIndices.end());
}

std::vector<real>
TimedElementSubdomainModifier::onGetTimes()
{
  std::vector<real> _times;
  return _times;
}

SubdomainID
TimedElementSubdomainModifier::onComputeSubdomainID(real /* t_from_exclusive */,
                                                    real /* t_to_inclusive */)
{
  SubdomainID _subdomain_id = _current_elem->subdomain_id();
  return _subdomain_id;
}

SubdomainID
TimedElementSubdomainModifier::computeSubdomainID()
{
  // did we advance to the next step?
  if (_current_step != _t_step)
  {
    _last_t = _current_t;
    _current_t = _t;
    _current_step = _t_step;
  }

  // get the new subdomain-id of the current element; provide the timespan to be considered
  SubdomainID _subdomain_id = onComputeSubdomainID(_last_t, _current_t);

  return _subdomain_id;
}
