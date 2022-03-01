//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayVacuumBC.h"

registerMooseObject("MooseApp", ArrayVacuumBC);

InputParameters
ArrayVacuumBC::validParams()
{
  InputParameters params = ArrayIntegratedBC::validParams();
  params.addParam<RealEigenVector>("alpha", "Ratio between directional gradient and solution");
  params.addClassDescription("Imposes the Robin boundary condition $\\partial_n "
                             "\\vec{u}=-\\frac{\\vec{\\alpha}}{2}\\vec{u}$.");
  return params;
}

ArrayVacuumBC::ArrayVacuumBC(const InputParameters & parameters)
  : ArrayIntegratedBC(parameters),
    _alpha(isParamValid("alpha") ? getParam<RealEigenVector>("alpha")
                                 : RealEigenVector::Ones(_count))
{
  if (_alpha.size() != _count)
    paramError(
        "alpha", "Number of values must equal number of variable components (", _count, ").");

  _alpha /= 2;
}

void
ArrayVacuumBC::computeQpResidual(RealEigenVector & residual)
{
  residual = _alpha.cwiseProduct(_u[_qp]) * _test[_i][_qp];
}

RealEigenVector
ArrayVacuumBC::computeQpJacobian()
{
  return _test[_i][_qp] * _phi[_j][_qp] * _alpha;
}
