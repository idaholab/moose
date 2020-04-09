//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSCompressibilityPenalty.h"

registerMooseObject("NavierStokesApp", INSCompressibilityPenalty);

InputParameters
INSCompressibilityPenalty::validParams()
{
  InputParameters params = Kernel::validParams();

  params.addClassDescription("The penalty term may be used when Dirichlet boundary condition is "
                             "applied to the entire boundary.");
  params.addParam<Real>("penalty", 1e-4, "Penalty value is used to relax the incompressibility");

  return params;
}

INSCompressibilityPenalty::INSCompressibilityPenalty(const InputParameters & parameters)
  : Kernel(parameters),
    // penalty value
    _penalty(getParam<Real>("penalty"))

{
}

Real
INSCompressibilityPenalty::computeQpResidual()
{
  // penalty*p*q
  return _penalty * _u[_qp] * _test[_i][_qp];
}

Real
INSCompressibilityPenalty::computeQpOffDiagJacobian(unsigned /* jvar */)
{
  // does not couple any variables
  return 0;
}

Real
INSCompressibilityPenalty::computeQpJacobian()
{
  // Derivative wrt to p
  return _penalty * _phi[_j][_qp] * _test[_i][_qp];
}
