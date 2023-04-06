//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PNSFVMomentumPressureFluxRZ.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", PNSFVMomentumPressureFluxRZ);
registerMooseObjectRenamed("NavierStokesApp",
                           PNSFVMomentumPressureRZ,
                           "05/01/2022 00:01",
                           PNSFVMomentumPressureFluxRZ);

InputParameters
PNSFVMomentumPressureFluxRZ::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params += INSFVMomentumResidualObject::validParams();
  params.addClassDescription(
      "Adds the porous $-p/r$ term into the radial component of the Navier-Stokes "
      "momentum equation for the problems in the RZ coordinate system when integrating by parts.");
  params.addParam<MooseFunctorName>(NS::pressure, "Pressure variable");
  params.addParam<MooseFunctorName>(NS::porosity, "Porosity variable");

  return params;
}

PNSFVMomentumPressureFluxRZ::PNSFVMomentumPressureFluxRZ(const InputParameters & params)
  : FVElementalKernel(params),
    INSFVMomentumResidualObject(*this),
    _p(getFunctor<ADReal>(NS::pressure)),
    _eps(getFunctor<ADReal>(NS::porosity))
{
}

ADReal
PNSFVMomentumPressureFluxRZ::computeQpResidual()
{
  mooseAssert(_subproblem.getCoordSystem(_current_elem->subdomain_id()) == Moose::COORD_RZ,
              "This object should only be active in an RZ coordinate system.");

  auto rz_radial_coord = _subproblem.getAxisymmetricRadialCoord();
  const auto state = determineState();

  return -_eps(makeElemArg(_current_elem), state) * _p(makeElemArg(_current_elem), state) /
         _q_point[_qp](rz_radial_coord);
}
