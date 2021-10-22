//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumPressureFluxRZ.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumPressureFluxRZ);
registerMooseObjectRenamed("NavierStokesApp",
                           PNSFVMomentumPressureRZ,
                           "05/01/2022 00:01",
                           PINSFVMomentumPressureFluxRZ);

InputParameters
PINSFVMomentumPressureFluxRZ::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription(
      "Adds the porous $-p/r$ term into the radial component of the Navier-Stokes "
      "momentum equation for the problems in the RZ coordinate system when integrating by parts.");
  params.addRequiredCoupledVar(NS::pressure, "Pressure variable");
  params.addRequiredCoupledVar(NS::porosity, "Porosity variable");

  return params;
}

PINSFVMomentumPressureFluxRZ::PINSFVMomentumPressureFluxRZ(const InputParameters & params)
  : FVElementalKernel(params), _p(adCoupledValue(NS::pressure)), _eps(coupledValue(NS::porosity))
{
}

ADReal
PINSFVMomentumPressureFluxRZ::computeQpResidual()
{
  mooseAssert(_subproblem.getCoordSystem(_current_elem->subdomain_id()) == Moose::COORD_RZ,
              "This object should only be active in an RZ coordinate system.");

  auto rz_radial_coord = _subproblem.getAxisymmetricRadialCoord();

  return -_eps[_qp] * _p[_qp] / _q_point[_qp](rz_radial_coord);
}
