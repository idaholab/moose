//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MassDiffusionSpeciesGasMixDGKernel.h"

registerMooseObject("ThermalHydraulicsApp", MassDiffusionSpeciesGasMixDGKernel);

InputParameters
MassDiffusionSpeciesGasMixDGKernel::validParams()
{
  InputParameters params = MassDiffusionBaseGasMixDGKernel::validParams();

  params.addClassDescription("Adds mass diffusion to the species equation for FlowChannelGasMix.");

  return params;
}

MassDiffusionSpeciesGasMixDGKernel::MassDiffusionSpeciesGasMixDGKernel(
    const InputParameters & parameters)
  : MassDiffusionBaseGasMixDGKernel(parameters)
{
}

ADReal
MassDiffusionSpeciesGasMixDGKernel::computeQpFlux() const
{
  Real dx, dx_side;
  computePositionChanges(dx, dx_side);

  const ADReal rho = linearlyInterpolate(_rho_elem[_qp], _rho_neig[_qp], dx, dx_side);
  const ADReal D = linearlyInterpolate(_D_elem[_qp], _D_neig[_qp], dx, dx_side);
  const ADReal dxi_dx = computeGradient(_mass_fraction_elem[_qp], _mass_fraction_neig[_qp], dx);

  return -rho * D * dxi_dx;
}
