//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMMixingConstantBeta.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("SubChannelApp", SCMMixingConstantBeta);

InputParameters
SCMMixingConstantBeta::validParams()
{
  InputParameters params = SCMMixingClosureBase::validParams();
  params.addClassDescription(
      "Class that models the turbulent mixing coefficient beta as a user defined constant.");
  params.addRequiredParam<Real>("beta", "Turbulent mixing parameter [-].");
  return params;
}

SCMMixingConstantBeta::SCMMixingConstantBeta(const InputParameters & parameters)
  : SCMMixingClosureBase(parameters), _beta(getParam<Real>("beta"))
{
}

Real
SCMMixingConstantBeta::computeMixingParameter(const unsigned int /*i_gap*/,
                                              const unsigned int /*iz*/,
                                              const bool /*sweep_flow*/) const
{
  return _beta;
}
