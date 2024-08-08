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
  return ElementSubdomainModifier::validParams();
}

TimedElementSubdomainModifier::TimedElementSubdomainModifier(const InputParameters & parameters)
  : ElementSubdomainModifier(parameters)
{
}

void
TimedElementSubdomainModifier::initialize()
{
  // clear number of moved elements
  ElementSubdomainModifier::initialize();

  // ask for all times (must NOT be sorted)
  const auto times = getTimes();

  // copy data to local storage
  std::size_t i = 0;
  for (const auto time : times)
    _times_and_indices.insert(TimeIndexPair{time, i++});
}
