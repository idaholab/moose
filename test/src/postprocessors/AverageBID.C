//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AverageBID.h"

#include "Assembly.h"

registerMooseObject("MooseTestApp", AverageBID);

template <>
InputParameters
validParams<AverageBID>()
{
  InputParameters params = validParams<SidePostprocessor>();
  return params;
}

AverageBID::AverageBID(const InputParameters & parameters) : SidePostprocessor(parameters) {}

void
AverageBID::initialize()
{
  _sum_bid = 0;
  _n_summands = 0;
}

void
AverageBID::execute()
{
  _sum_bid += _current_boundary_id;
  ++_n_summands;
}

void
AverageBID::finalize()
{
  gatherSum(_sum_bid);
  gatherSum(_n_summands);
}

Real
AverageBID::getValue()
{
  return _sum_bid / _n_summands;
}

void
AverageBID::threadJoin(const UserObject & y)
{
  const AverageBID & pps = static_cast<const AverageBID &>(y);
  _sum_bid += pps._sum_bid;
  _n_summands += pps._n_summands;
}
