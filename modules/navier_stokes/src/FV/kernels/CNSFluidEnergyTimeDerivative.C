//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSFluidEnergyTimeDerivative.h"
#include "NS.h"

namespace nms = NS;

registerADMooseObject("NavierStokesApp", CNSFluidEnergyTimeDerivative);

defineADValidParams(CNSFluidEnergyTimeDerivative,
                    TimeDerivativeKernel,
                    params.addClassDescription("Time derivative $\\epsilon\\frac{\\partial(\\rho_fE_f)}{\\partial t}$ "
                      "in the Navier-Stokes and Euler fluid energy conservation equations."););

CNSFluidEnergyTimeDerivative::CNSFluidEnergyTimeDerivative(
    const InputParameters & parameters)
  : TimeDerivativeKernel(parameters),
    _drhoE_dt(getADMaterialProperty<Real>(nms::time_deriv(nms::rho_et)))
{
}

ADReal
CNSFluidEnergyTimeDerivative::timeDerivative()
{
  return _drhoE_dt[_qp];
}
