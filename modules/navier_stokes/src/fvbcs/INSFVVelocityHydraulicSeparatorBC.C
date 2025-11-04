//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVVelocityHydraulicSeparatorBC.h"
#include "InputParameters.h"

registerMooseObject("NavierStokesApp", INSFVVelocityHydraulicSeparatorBC);

InputParameters
INSFVVelocityHydraulicSeparatorBC::validParams()
{
  auto params = INSFVFluxBC::validParams();
  params.addClassDescription("A separator boundary condition that prevents any type of momentum "
                             "flux and can be used within the domain.");
  return params;
}

INSFVVelocityHydraulicSeparatorBC::INSFVVelocityHydraulicSeparatorBC(const InputParameters & params)
  : INSFVFluxBC(params), INSFVHydraulicSeparatorInterface(this)
{
}
