//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVMomentumPressureRZ.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", PCNSFVMomentumPressureRZ);

InputParameters
PCNSFVMomentumPressureRZ::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription(
      "Adds the porous $-p/r$ term into the radial component of the Navier-Stokes "
      "momentum equation for the problems in the RZ coordinate system.");
  return params;
}

PCNSFVMomentumPressureRZ::PCNSFVMomentumPressureRZ(const InputParameters & params)
  : FVElementalKernel(params),
    _p(getADMaterialProperty<Real>(NS::pressure)),
    _eps(getMaterialProperty<Real>(NS::porosity))
{
}

ADReal
PCNSFVMomentumPressureRZ::computeQpResidual()
{
  mooseAssert(_subproblem.getCoordSystem(_current_elem->subdomain_id()) == Moose::COORD_RZ,
              "This object should only be active in an RZ coordinate system.");

  auto rz_radial_coord = _subproblem.getAxisymmetricRadialCoord();

  return -_eps[_qp] * _p[_qp] / _q_point[_qp](rz_radial_coord);
}
