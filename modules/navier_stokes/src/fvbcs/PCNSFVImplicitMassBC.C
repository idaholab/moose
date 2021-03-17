//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVImplicitMassBC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PCNSFVImplicitMassBC);

InputParameters
PCNSFVImplicitMassBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription("Implicit mass BC.");
  return params;
}

PCNSFVImplicitMassBC::PCNSFVImplicitMassBC(const InputParameters & params)
  : FVFluxBC(params), _mass_flux(getADMaterialProperty<RealVectorValue>(NS::mass_flux))
{
}

ADReal
PCNSFVImplicitMassBC::computeQpResidual()
{
  mooseAssert(this->hasBlocks(_face_info->elem().subdomain_id()), "Checking block restriction");

  return _normal * _mass_flux[_qp];
}
