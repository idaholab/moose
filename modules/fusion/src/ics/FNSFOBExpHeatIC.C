//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FNSFOBExpHeatIC.h"
#include "FNSFUtils.h"

using namespace FNSF;

registerMooseObject("FusionApp", FNSFOBExpHeatIC);

InputParameters
FNSFOBExpHeatIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addClassDescription(
      "Sets values using the functional form y = A*exp(-k*d) where d is the depth measured from "
      "the last closed flux surface in meters. This IC is only intended to be used for the "
      "outboard blanket of the FNSF model.");
  params.addParam<Real>("A", 2.6464e7, "The scaling term");
  params.addParam<Real>("k", 8.8698, "The decay rate");
  return params;
}

FNSFOBExpHeatIC::FNSFOBExpHeatIC(const InputParameters & parameters)
  : InitialCondition(parameters), _A(getParam<Real>("A")), _k(getParam<Real>("k"))
{
}

Real
FNSFOBExpHeatIC::value(const Point & p)
{
  Real r = std::sqrt(p(0) * p(0) + p(1) * p(1));
  Real z = p(2);

  std::pair<Real, Real> xi_depth = find_xi_depth(r, z);
  Real xi = xi_depth.first;
  Real depth = xi_depth.second;

  return _A * std::exp(-_k * depth);
}
