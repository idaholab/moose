//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PFFracBulkRate.h"
#include "MathUtils.h"

template <>
InputParameters
validParams<PFFracBulkRate>()
{
  InputParameters params = validParams<KernelValue>();
  params.addClassDescription(
      "Kernel to compute bulk energy contribution to damage order parameter residual equation");
  params.addRequiredParam<Real>("l", "Width of the smooth crack representation");
  params.addRequiredParam<Real>(
      "visco", "Viscosity parameter, which reflects the transition right at crack stress");
  params.addRequiredParam<MaterialPropertyName>(
      "gc_prop_var", "Material property which provides the maximum stress/crack stress");
  params.addRequiredParam<MaterialPropertyName>(
      "G0_var", "Material property name with undamaged strain energy driving damage (G0_pos)");
  params.addParam<MaterialPropertyName>(
      "dG0_dstrain_var", "Material property name with derivative of G0_pos with strain");
  params.addRequiredCoupledVar("beta", "Laplacian of the kernel variable");

  params.addCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");

  return params;
}

PFFracBulkRate::PFFracBulkRate(const InputParameters & parameters)
  : KernelValue(parameters),
    _gc_prop(getMaterialProperty<Real>("gc_prop_var")),
    _G0_pos(getMaterialProperty<Real>("G0_var")),
    _dG0_pos_dstrain(isParamValid("dG0_dstrain_var")
                         ? &getMaterialProperty<RankTwoTensor>("dG0_dstrain_var")
                         : NULL),
    _betaval(coupledValue("beta")),
    _beta_var(coupled("beta")),
    _ndisp(coupledComponents("displacements")),
    _disp_var(_ndisp),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _l(getParam<Real>("l")),
    _visco(getParam<Real>("visco"))
{
  mooseDeprecated("Use 'SplitPFFractureBulkRate'. Parameter name changes: l->width, "
                  "visco->viscosity, gc_prop_var->gc, G0_var->G0, dG0_dstrain_var->dG0_dstrain");
  for (unsigned int i = 0; i < _ndisp; ++i)
    _disp_var[i] = coupled("displacements", i);
}

Real
PFFracBulkRate::precomputeQpResidual()
{
  const Real gc = _gc_prop[_qp];
  const Real c = _u[_qp];
  const Real x = _l * _betaval[_qp] + 2.0 * (1.0 - c) * _G0_pos[_qp] / gc - c / _l;

  return -((std::abs(x) + x) / 2.0) / _visco;
}

Real
PFFracBulkRate::precomputeQpJacobian()
{
  const Real gc = _gc_prop[_qp];
  const Real c = _u[_qp];
  const Real x = _l * _betaval[_qp] + 2.0 * (1.0 - c) * _G0_pos[_qp] / gc - c / _l;

  return (MathUtils::sign(x) + 1.0) / 2.0 * (2.0 * _G0_pos[_qp] / gc + 1.0 / _l) / _visco *
         _phi[_j][_qp];
}

Real
PFFracBulkRate::computeQpOffDiagJacobian(unsigned int jvar)
{
  unsigned int c_comp;
  bool disp_flag = false;

  const Real c = _u[_qp];
  const Real gc = _gc_prop[_qp];

  const Real x = _l * _betaval[_qp] + 2.0 * (1.0 - c) * (_G0_pos[_qp] / gc) - c / _l;

  const Real signx = MathUtils::sign(x);

  Real xfacbeta = -((signx + 1.0) / 2.0) / _visco * _l;
  Real xfac = -((signx + 1.0) / 2.0) / _visco * 2.0 * (1.0 - c) / gc;

  // Contribution of auxiliary variable to off diag Jacobian of c
  for (unsigned int k = 0; k < _ndisp; ++k)
  {
    if (jvar == _beta_var)
      return xfacbeta * _phi[_j][_qp] * _test[_i][_qp];
    else if (jvar == _disp_var[k])
    {
      c_comp = k;
      disp_flag = true;
    }
    else
      return 0.0;
  }

  // Contribution of displacements to off diag Jacobian of c
  if (disp_flag && _dG0_pos_dstrain != NULL)
  {
    Real val = 0.0;
    for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
      val += ((*_dG0_pos_dstrain)[_qp](c_comp, i) + (*_dG0_pos_dstrain)[_qp](i, c_comp)) / 2.0 *
             _grad_phi[_j][_qp](i);

    return xfac * val * _test[_i][_qp];
  }

  return 0.0;
}
