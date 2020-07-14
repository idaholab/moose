//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeDerivativeKernel.h"
#include "NS.h"

namespace nms = NS;

// Full specialization of the validParams function for this object
defineADValidParams(TimeDerivativeKernel,
                    CNSKernel,
                    params.addRequiredCoupledVar(nms::porosity, "porosity");
                    params.addClassDescription("Base kernel for time derivatives of conserved fields"););

TimeDerivativeKernel::TimeDerivativeKernel(const InputParameters & parameters)
  : CNSKernel(parameters),
    _eps(coupledValue(nms::porosity))
{
}

ADReal
TimeDerivativeKernel::weakResidual()
{
  return _eps[_qp] * timeDerivative() * _test[_i][_qp];
}

ADReal
TimeDerivativeKernel::strongResidual()
{
  return _eps[_qp] * timeDerivative();
}
