//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearWCNSFVMomentumFlux.h"
#include "Assembly.h"
#include "SubProblem.h"

registerMooseObject("NavierStokesApp", LinearWCNSFVMomentumFlux);

InputParameters
LinearWCNSFVMomentumFlux::validParams()
{
  InputParameters params = LinearFVFluxKernel::validParams();
  params.addClassDescription("Represents the matrix and right hand side contributions of the "
                             "stress and advection terms of the momentum equation.");
  params.addRequiredParam<MooseFunctorName>("advecting_velocity", "Advection velocity");
  params += Moose::FV::advectedInterpolationParameter();
  return params;
}

LinearWCNSFVMomentumFlux::LinearWCNSFVMomentumFlux(const InputParameters & params)
  : LinearFVFluxKernel(params),
    _vel_provider(getUserObject<INSFVRhieChowInterpolatorSegregated>("rhie_chow_user_object"))

{
  Moose::FV::setInterpolationMethod(*this, _advected_interp_method, "advected_interp_method");
}

void
LinearWCNSFVMomentumFlux::initialSetup()
{
}

Real
LinearWCNSFVMomentumFlux::computeElemMatrixContribution()
{
  return 0.0;
}

Real
LinearWCNSFVMomentumFlux::computeNeighborMatrixContribution()
{
  return 0.0;
}

Real
LinearWCNSFVMomentumFlux::computeElemRightHandSideContribution()
{
  return 0.0;
}

Real
LinearWCNSFVMomentumFlux::computeNeighborRightHandSideContribution()
{
  return 0.0;
}

Real
LinearWCNSFVMomentumFlux::computeBoundaryMatrixContribution(
    const LinearFVBoundaryCondition & /*bc*/)
{
  return 0.0;
}

Real
LinearWCNSFVMomentumFlux::computeBoundaryRHSContribution(const LinearFVBoundaryCondition & /*bc*/)
{
  return 0.0;
}
