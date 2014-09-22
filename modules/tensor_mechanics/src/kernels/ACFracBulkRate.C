#include "ACFracBulkRate.h"

/*
Formulation: Miehe et. al., Int. J. Num. Methods Engg., 2010, 83. 1273-1311
Equation 63-3
For Mat Props user FractureACBulkMatRate or similar
*/


template<>
InputParameters validParams<ACFracBulkRate>()
{
  InputParameters params = validParams<KernelValue>();

  params.addParam<std::string>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<Real>("l", 1.0, "Interface width");
  params.addParam<Real>("visco", 1e-6, "Viscosity");
  params.addRequiredCoupledVar("beta", "Penalty variable");
  params.addCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");

  return params;
}

ACFracBulkRate::ACFracBulkRate(const std::string & name,
                               InputParameters parameters):
    KernelValue(name,parameters),
    _mob_name(getParam<std::string>("mob_name")),
    _L(getMaterialProperty<Real>(_mob_name)),
    _gc_prop_tens(getMaterialProperty<Real>("gc_prop_tens")),
    _G0_pos(getMaterialProperty<Real>("G0_pos")),
    _dG0_pos_dstrain(getMaterialProperty<RankTwoTensor>("dG0_pos_dstrain")),
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
ACFracBulkRate::computeDFDOP(PFFunctionType type)
{
  switch (type)
  {
    case Residual:
    {
      Real c = _u[_qp];


      Real gc_tens=_gc_prop_tens[_qp];


      Real x =   _betaval[_qp]
               + 2.0*(1.0-c) * _G0_pos[_qp]/gc_tens - c/_l;
      return -((std::abs(x)+x)/2.0) / _visco;
    }

    case Jacobian:
    {
      Real c = _u[_qp];


      Real gc_tens = _gc_prop_tens[_qp];


      Real x = _betaval[_qp] + 2.0 * (1.0 - c) * _G0_pos[_qp] / gc_tens - c / _l;
      Real signx = x > 0.0 ? 1.0 : -1.0;
      return (signx + 1.0)/2.0 *
                 (2.0 * _G0_pos[_qp]/gc_tens + 1.0/_l)/_visco;
    }
  }

  mooseError("Invalid type passed in");
}

Real
ACFracBulkRate::precomputeQpResidual()
{
  Real dFdeta = computeDFDOP(Residual);
  return  _L[_qp] * dFdeta;
}

Real
ACFracBulkRate::precomputeQpJacobian()
{
  Real dFdeta = computeDFDOP(Jacobian);
  return _L[_qp] * dFdeta * _phi[_j][_qp];
}

Real
ACFracBulkRate::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real c = _u[_qp];


  Real gc_tens = _gc_prop_tens[_qp];


  Real x = _betaval[_qp]+2.0*(1.0-c)*(_G0_pos[_qp]/gc_tens)-c/_l;
  Real signx = x > 0.0 ? 1.0 : -1.0;

  Real xfacbeta = -((signx + 1.0)/2.0) / _visco;
  Real xfac = -((signx + 1.0)/2.0) / _visco * 2.0 * (1.0 - c) / gc_tens;

  if (jvar == _beta_var)
    return _L[_qp] * xfacbeta * _phi[_j][_qp] * _test[_i][_qp];
  else if (_xdisp_coupled && jvar == _xdisp_var)
  {
    unsigned int c_comp = 0;
    Real val = 0.0;

    for (unsigned int i = 0; i < 3; ++i)
      val +=  (_dG0_pos_dstrain[_qp](c_comp,i)
             + _dG0_pos_dstrain[_qp](i,c_comp))/2.0 * _grad_phi[_j][_qp](i);

    return _L[_qp] * xfac * val * _test[_i][_qp];
  }
  else if (_ydisp_coupled && jvar == _ydisp_var)
  {
    unsigned int c_comp = 1;
    Real val = 0.0;

    for (unsigned int i = 0; i < 3; ++i)
      val +=  (_dG0_pos_dstrain[_qp](c_comp,i)
             + _dG0_pos_dstrain[_qp](i,c_comp))/2.0 * _grad_phi[_j][_qp](i);

    return _L[_qp] * xfac * val * _test[_i][_qp];
  }
  else if (_zdisp_coupled && jvar == _zdisp_var)
  {
    unsigned int c_comp = 2;
    Real val = 0.0;

    for (unsigned int i = 0; i < 3; ++i)
      val +=  (_dG0_pos_dstrain[_qp](c_comp,i)
             + _dG0_pos_dstrain[_qp](i,c_comp))/2.0 * _grad_phi[_j][_qp](i);

    return _L[_qp] * xfac * val * _test[_i][_qp];
  }
  else
    return 0.0;
}
