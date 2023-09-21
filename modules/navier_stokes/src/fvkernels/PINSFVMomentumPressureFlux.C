//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumPressureFlux.h"
#include "PINSFVSuperficialVelocityVariable.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumPressureFlux);

InputParameters
PINSFVMomentumPressureFlux::validParams()
{
  auto params = INSFVMomentumPressureFlux::validParams();
  params.addClassDescription("Momentum pressure term eps grad_P, as a flux kernel "
                             "using the divergence theoreom, in the porous media "
                             "incompressible Navier-Stokes momentum equation. This kernel "
                             "is also executed on boundaries.");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "Porosity functor");
  params.set<bool>("force_boundary_execution") = true;
  return params;
}

PINSFVMomentumPressureFlux::PINSFVMomentumPressureFlux(const InputParameters & params)
  : INSFVMomentumPressureFlux(params), _eps(getFunctor<ADReal>(NS::porosity))
{
  if (!dynamic_cast<PINSFVSuperficialVelocityVariable *>(&_var))
    mooseError("PINSFVMomentumPressureFlux may only be used with a superficial velocity, "
               "of variable type PINSFVSuperficialVelocityVariable.");
}
