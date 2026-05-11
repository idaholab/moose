//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RhieChowMassFlowRate.h"

registerMooseObject("NavierStokesApp", RhieChowMassFlowRate);

InputParameters
RhieChowMassFlowRate::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addClassDescription("Integrates the mass flux stored by a linear segregated Rhie-Chow "
                             "user object over a boundary.");
  params.addRequiredParam<UserObjectName>(
      "rhie_chow_user_object", "The Rhie-Chow user object providing the face mass flux.");
  return params;
}

RhieChowMassFlowRate::RhieChowMassFlowRate(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _mass_flux_provider(getUserObject<RhieChowMassFlux>("rhie_chow_user_object"))
{
  _qp_integration = false;
}

Real
RhieChowMassFlowRate::computeQpIntegral()
{
  mooseError("RhieChowMassFlowRate only supports finite volume FaceInfo integration.");
}

Real
RhieChowMassFlowRate::computeFaceInfoIntegral(const FaceInfo * fi)
{
  mooseAssert(fi, "We should have a FaceInfo.");
  return _mass_flux_provider.getMassFlux(*fi);
}
