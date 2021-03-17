//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVImplicitFluidEnergyAdvectionBC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PCNSFVImplicitFluidEnergyAdvectionBC);

InputParameters
PCNSFVImplicitFluidEnergyAdvectionBC::validParams()
{
  InputParameters params = PCNSFVImplicitMassBC::validParams();
  params.addClassDescription("Implicit fluid energy advection BC.");
  return params;
}

PCNSFVImplicitFluidEnergyAdvectionBC::PCNSFVImplicitFluidEnergyAdvectionBC(
    const InputParameters & params)
  : PCNSFVImplicitMassBC(params), _ht(getADMaterialProperty<Real>(NS::specific_total_enthalpy))
{
}

ADReal
PCNSFVImplicitFluidEnergyAdvectionBC::computeQpResidual()
{
  return PCNSFVImplicitFluidEnergyAdvectionBC::computeQpResidual() * _ht[_qp];
}
