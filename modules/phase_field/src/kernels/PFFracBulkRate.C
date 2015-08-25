/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PFFracBulkRate.h"

template<>
InputParameters validParams<PFFracBulkRate>()
{
  InputParameters params = validParams<KernelValue>();
  params.addClassDescription("Kernel to compute bulk energy contribution to damage order parameter residual equation");
  params.addRequiredParam<Real>("l","Interface width");
  params.addRequiredParam<Real>("visco","Viscosity parameter");
  params.addRequiredParam<MaterialPropertyName>("gc_prop_var", "Material property name with gc value");
  params.addRequiredParam<MaterialPropertyName>("G0_var", "Material property name with undamaged strain energy driving damage (G0_pos)");
  params.addParam<MaterialPropertyName>("dG0_dstrain_var", "Material property name with derivative of G0_pos with strain");
  params.addRequiredCoupledVar("beta", "Auxiliary variable");
  params.addCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");

  return params;
}

PFFracBulkRate::PFFracBulkRate(const InputParameters & parameters):
  KernelValue(parameters),
  _gc_prop(getMaterialProperty<Real>("gc_prop_var")),
  _G0_pos(getMaterialProperty<Real>("G0_var")),
  _dG0_pos_dstrain(isParamValid("dG0_dstrain_var") ? &getMaterialProperty<RankTwoTensor>("dG0_dstrain_var"): NULL),
  _betaval(coupledValue("beta")),
  _beta_var(coupled("beta")),
  _xdisp_coupled(isCoupled("disp_x")),
  _ydisp_coupled(isCoupled("disp_y")),
  _zdisp_coupled(isCoupled("disp_z")),
  _xdisp_var(_xdisp_coupled ? coupled("disp_x") : 0),
  _ydisp_var(_ydisp_coupled ? coupled("disp_y") : 0),
  _zdisp_var(_zdisp_coupled ? coupled("disp_z") : 0),
  _l(getParam<Real>("l")),
  _visco(getParam<Real>("visco"))
{
}

Real
PFFracBulkRate::computeDFDOP(PFFunctionType type)
{
  Real gc = _gc_prop[_qp];

  switch (type)
  {
    case Residual:
    {
      //Order parameter for damage variable
      Real c = _u[_qp];
      Real x =   _l * _betaval[_qp] + 2.0*(1.0-c) * _G0_pos[_qp]/gc - c/_l;

      return -(( std::abs(x) + x )/2.0)/_visco;
    }
    case Jacobian:
    {
      Real c = _u[_qp];
      Real x = _l * _betaval[_qp] + 2.0 * (1.0 - c) * _G0_pos[_qp] / gc - c / _l;
      Real signx = x > 0.0 ? 1.0 : -1.0;
      return (signx + 1.0)/2.0 * (2.0 * _G0_pos[_qp]/gc + 1.0/_l)/_visco;
    }
    default:
      mooseError("PFFracBulkRate: Invalid type passed - case must be either Residual or Jacobian");
  }
  mooseError("PFFracBulkRate: Invalid type passed");
}

Real
PFFracBulkRate::precomputeQpResidual()
{
  Real dFdeta = computeDFDOP(Residual);
  return  dFdeta;
}

Real
PFFracBulkRate::precomputeQpJacobian()
{
  Real dFdeta = computeDFDOP(Jacobian);
  return dFdeta * _phi[_j][_qp];
}

Real
PFFracBulkRate::computeQpOffDiagJacobian(unsigned int jvar)
{
  unsigned int c_comp;
  bool disp_flag = false;

  Real c = _u[_qp];
  Real gc = _gc_prop[_qp];

  Real x = _l * _betaval[_qp] + 2.0*(1.0-c) * (_G0_pos[_qp]/gc) - c/_l;
  Real signx = x > 0.0 ? 1.0 : -1.0;

  Real xfacbeta = -((signx + 1.0)/2.0) / _visco * _l;
  Real xfac = -((signx + 1.0)/2.0) / _visco * 2.0 * (1.0 - c) / gc;

  if (jvar == _beta_var)
    //Contribution of auxiliary variable to off diag Jacobian of c
    return xfacbeta * _phi[_j][_qp] * _test[_i][_qp];
  else if (_xdisp_coupled && jvar == _xdisp_var)
  {
    c_comp = 0;
    disp_flag = true;
  }
  else if (_ydisp_coupled && jvar == _ydisp_var)
  {
    c_comp = 1;
    disp_flag = true;
  }
  else if (_zdisp_coupled && jvar == _zdisp_var)
  {
    c_comp = 2;
    disp_flag = true;
  }
  else
    return 0.0;
  //Contribution of displacements to off diag Jacobian of c
  if (disp_flag && _dG0_pos_dstrain != NULL)
  {
    Real val = 0.0;

    for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
      val +=  ((*_dG0_pos_dstrain)[_qp](c_comp,i) + (*_dG0_pos_dstrain)[_qp](i,c_comp))/2.0 * _grad_phi[_j][_qp](i);

    return xfac * val * _test[_i][_qp];
  }

  return 0.0;
}

