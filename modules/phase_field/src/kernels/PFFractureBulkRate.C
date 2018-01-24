//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PFFractureBulkRate.h"
#include "MooseVariable.h"
#include "RankTwoTensor.h"
#include "MathUtils.h"

template <>
InputParameters
validParams<PFFractureBulkRate>()
{
  InputParameters params = validParams<PFFractureBulkRateBase>();
  params.addClassDescription(
      "Kernel to compute bulk energy contribution to damage order parameter residual equation");
  return params;
}

PFFractureBulkRate::PFFractureBulkRate(const InputParameters & parameters)
  : PFFractureBulkRateBase(parameters), _second_u(second())
{
}

Real
PFFractureBulkRate::computeQpResidual()
{
  const Real & gc = _gc_prop[_qp];
  const Real & c = _u[_qp];
  const Real beta = _second_u[_qp].tr();
  const Real x2 = 2.0 * (1.0 - c) * _G0_pos[_qp] / gc - c / _width;
  const Real x1 = _width * beta;

  /**
   * This if implements the <...>+ positive part operator. To determine if the
   * expression is positive we use the original form that is _not_ integrated by parts.
   * The residual we return, however, replaces x1 with the expression that is integrated by parts
   * once (_second_u[_qp].tr() * _test[_i][_qp] -> -_grad_u[_qp] * _grad_test[_i][_qp]).
   * This allows the user to use H1 (square integrable first derivatives) shape
   * functions (such as Lagrange).
   */
  if (x1 + x2 > 0)
    return -(-_width * _grad_u[_qp] * _grad_test[_i][_qp] + x2 * _test[_i][_qp]) / _viscosity;

  return 0.0;
}

Real
PFFractureBulkRate::computeQpJacobian()
{
  const Real & gc = _gc_prop[_qp];
  const Real & c = _u[_qp];
  const Real beta = _second_u[_qp].tr();
  const Real x = _width * beta + 2.0 * (1.0 - c) * _G0_pos[_qp] / gc - c / _width;

  if (x > 0)
  {
    const Real dx =
        -_width * _grad_phi[_j][_qp] * _grad_test[_i][_qp] -
        (2.0 * _phi[_j][_qp] * _G0_pos[_qp] / gc - _phi[_j][_qp] / _width) * _test[_i][_qp];

    return -dx / _viscosity;
  }

  return 0.0;
}

Real
PFFractureBulkRate::computeQpOffDiagJacobian(unsigned int jvar)
{
  // bail out early if no stress derivative has been provided
  if (_dG0_pos_dstrain == NULL)
    return 0.0;

  // displacement variables
  unsigned int c_comp = 0;
  for (; c_comp < _ndisp; ++c_comp)
    if (jvar == _disp_var[c_comp])
      break;

  // Contribution of displacements to off-diagonal Jacobian of c
  if (c_comp < _ndisp)
  {
    const Real & c = _u[_qp];
    const Real & gc = _gc_prop[_qp];
    const Real beta = _second_u[_qp].tr();
    const Real x1 = _width * beta;
    const Real x2 = 2.0 * (1.0 - c) * _G0_pos[_qp] / gc - c / _width;
    if (x1 + x2 > 0)
    {
      const Real xfac = -(-_width * _grad_u[_qp] * _grad_test[_i][_qp] + x2 * _test[_i][_qp]) /
                        _viscosity * 2.0 * (1.0 - c) / gc;

      Real val = 0.0;
      for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
        val += ((*_dG0_pos_dstrain)[_qp](c_comp, i) + (*_dG0_pos_dstrain)[_qp](i, c_comp)) / 2.0 *
               _grad_phi[_j][_qp](i);

      return xfac * val;
    }
  }

  return 0.0;
}
