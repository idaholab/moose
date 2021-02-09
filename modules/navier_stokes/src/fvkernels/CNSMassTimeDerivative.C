//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSMassTimeDerivative.h"
#include "NS.h"

namespace nms = NS;

// Full specialization of the validParams function for this object
registerADMooseObject("NavierStokesApp", CNSMassTimeDerivative);

defineADValidParams(CNSMassTimeDerivative,
                    TimeDerivativeKernel,
                    params.addClassDescription("Time derivative $\\epsilon\\frac{\\partial\\rho_f}{\\partial t}$ "
                                               "in the mass conservation equation."););

CNSMassTimeDerivative::CNSMassTimeDerivative(const InputParameters & parameters)
  : TimeDerivativeKernel(parameters),
    _drho_dt(getADMaterialProperty<Real>(nms::time_deriv(nms::density)))
{
}

ADReal
CNSMassTimeDerivative::timeDerivative()
{
  return _drho_dt[_qp];
}
