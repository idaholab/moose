/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SplitPFFractureBulkRate.h"
#include "MathUtils.h"

template <>
InputParameters
validParams<SplitPFFractureBulkRate>()
{
  InputParameters params = validParams<KernelValue>();
  params.addClassDescription(
      "Kernel to compute bulk energy contribution to damage order parameter residual equation");
  params.addRequiredParam<Real>("width", "Width of the smooth crack representation");
  params.addRequiredParam<Real>(
      "viscosity", "Viscosity parameter, which reflects the transition right at crack stress");
  params.addRequiredParam<MaterialPropertyName>(
      "gc", "Material property which provides the maximum stress/crack stress");
  params.addRequiredParam<MaterialPropertyName>(
      "G0", "Material property name with undamaged strain energy driving damage (G0_pos)");
  params.addParam<MaterialPropertyName>(
      "dG0_dstrain", "Material property name with derivative of G0_pos with strain");
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

SplitPFFractureBulkRate::SplitPFFractureBulkRate(const InputParameters & parameters)
  : KernelValue(parameters),
    _gc_prop(getMaterialProperty<Real>("gc")),
    _G0_pos(getMaterialProperty<Real>("G0")),
    _dG0_pos_dstrain(
        isParamValid("dG0_dstrain") ? &getMaterialProperty<RankTwoTensor>("dG0_dstrain") : NULL),
    _beta(coupledValue("beta")),
    _beta_var(coupled("beta")),
    _ndisp(coupledComponents("displacements")),
    _disp_var(_ndisp),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _width(getParam<Real>("width")),
    _viscosity(getParam<Real>("viscosity"))
{
  for (unsigned int i = 0; i < _ndisp; ++i)
    _disp_var[i] = coupled("displacements", i);
}

Real
SplitPFFractureBulkRate::precomputeQpResidual()
{
  const Real gc = _gc_prop[_qp];
  const Real c = _u[_qp];
  const Real x = _width * _beta[_qp] + 2.0 * (1.0 - c) * _G0_pos[_qp] / gc - c / _width;

  return -MathUtils::positivePart(x) / _viscosity;
}

Real
SplitPFFractureBulkRate::precomputeQpJacobian()
{
  const Real gc = _gc_prop[_qp];
  const Real c = _u[_qp];
  const Real x = _width * _beta[_qp] + 2.0 * (1.0 - c) * _G0_pos[_qp] / gc - c / _width;
  const Real dx = -2.0 * _G0_pos[_qp] / gc - 1.0 / _width;

  return -MathUtils::heavyside(x) / _viscosity * dx * _phi[_j][_qp];
}

Real
SplitPFFractureBulkRate::computeQpOffDiagJacobian(unsigned int jvar)
{

  const Real & c = _u[_qp];
  const Real & gc = _gc_prop[_qp];
  const Real x = _width * _beta[_qp] + 2.0 * (1.0 - c) * (_G0_pos[_qp] / gc) - c / _width;
  const Real xfacbeta = -MathUtils::heavyside(x) / _viscosity * _width;

  // Contribution of Laplacian split variable
  if (jvar == _beta_var)
    return xfacbeta * _phi[_j][_qp] * _test[_i][_qp];

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
