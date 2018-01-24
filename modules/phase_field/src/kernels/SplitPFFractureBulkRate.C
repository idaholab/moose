//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SplitPFFractureBulkRate.h"
#include "RankTwoTensor.h"
#include "MathUtils.h"

template <>
InputParameters
validParams<SplitPFFractureBulkRate>()
{
  InputParameters params = validParams<PFFractureBulkRateBase>();
  params.addClassDescription(
      "Kernel to compute bulk energy contribution to damage order parameter residual equation");
  params.addRequiredCoupledVar("beta", "Laplacian of the kernel variable");
  return params;
}

SplitPFFractureBulkRate::SplitPFFractureBulkRate(const InputParameters & parameters)
  : PFFractureBulkRateBase(parameters), _beta(coupledValue("beta")), _beta_var(coupled("beta"))
{
}

Real
SplitPFFractureBulkRate::computeQpResidual()
{
  const Real & gc = _gc_prop[_qp];
  const Real & c = _u[_qp];
  const Real x = _width * _beta[_qp] + 2.0 * (1.0 - c) * _G0_pos[_qp] / gc - c / _width;

  return -MathUtils::positivePart(x) / _viscosity * _test[_i][_qp];
}

Real
SplitPFFractureBulkRate::computeQpJacobian()
{
  const Real & gc = _gc_prop[_qp];
  const Real & c = _u[_qp];
  const Real x = _width * _beta[_qp] + 2.0 * (1.0 - c) * _G0_pos[_qp] / gc - c / _width;
  const Real dx = -2.0 * _G0_pos[_qp] / gc - 1.0 / _width;

  return -MathUtils::heavyside(x) / _viscosity * dx * _phi[_j][_qp] * _test[_i][_qp];
}

Real
SplitPFFractureBulkRate::computeQpOffDiagJacobian(unsigned int jvar)
{

  const Real & c = _u[_qp];
  const Real & gc = _gc_prop[_qp];
  const Real x = _width * _beta[_qp] + 2.0 * (1.0 - c) * (_G0_pos[_qp] / gc) - c / _width;

  // Contribution of Laplacian split variable
  if (jvar == _beta_var)
  {
    const Real xfacbeta = -MathUtils::heavyside(x) / _viscosity * _width;
    return xfacbeta * _phi[_j][_qp] * _test[_i][_qp];
  }

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
    const Real xfac = -MathUtils::heavyside(x) / _viscosity * 2.0 * (1.0 - c) / gc;
    Real val = 0.0;
    for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
      val += ((*_dG0_pos_dstrain)[_qp](c_comp, i) + (*_dG0_pos_dstrain)[_qp](i, c_comp)) / 2.0 *
             _grad_phi[_j][_qp](i);

    return xfac * val * _test[_i][_qp];
  }

  return 0.0;
}
