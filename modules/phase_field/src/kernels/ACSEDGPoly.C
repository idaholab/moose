/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACSEDGPoly.h"
#include "Material.h"

template<>
InputParameters validParams<ACSEDGPoly>()
{
  InputParameters params = ACBulk<Real>::validParams();
  params.addClassDescription("Stored Energy contribution to grain growth");
  params.addRequiredCoupledVar("v", "Array of coupled variable names");
  params.addRequiredParam<unsigned int>("ndef", "Number of OP representing deformed grains");
  return params;
}

ACSEDGPoly::ACSEDGPoly(const InputParameters & parameters) :
    ACBulk<Real>(parameters),
    _ncrys(coupledComponents("v")),
    _vals(_ncrys),
    _vals_var(_ncrys),
    _beta(getMaterialProperty<Real>("beta")),
    _rho_eff(getMaterialProperty<Real>("rho_eff")),
    _Disloc_Den_i(getMaterialProperty<Real>("Disloc_Den_i")),
    _ndef(getParam<unsigned int>("ndef")),
    _op_index(getParam<unsigned int>("op_index"))
{
  // Loop through grains and load coupled variables into the arrays
  for (unsigned int i = 0; i < _ncrys; ++i)
  {
    _vals[i] = &coupledValue("v", i);
    _vals_var[i] = coupled("v", i);
  }
}

Real
ACSEDGPoly::computeDFDOP(PFFunctionType type)
{
  Real SumEtaj = 0.0;
  for (unsigned int i = 0; i < _ncrys; ++i)
    SumEtaj += (*_vals[i])[_qp] * (*_vals[i])[_qp];

  // Add the current OP to the sum
  Real SumEtai2 = SumEtaj + _u[_qp] * _u[_qp];
  // Dislocation density in deformed grains
  Real rho_i = _Disloc_Den_i[_qp];
  // undeformed grains are dislocation-free
  if (_op_index >= _ndef)
    rho_i = 0.0;
  // Calculate the contributions of the deformation energy to the residual and Jacobian
  Real drho_eff_detai = 2.0 * _u[_qp] * (rho_i - _rho_eff[_qp]) / SumEtai2;

  // Calculate the Stored Energy contribution to either the residual or Jacobian of the grain growth free energy
  switch (type)
  {
    case Residual:
      return   _beta[_qp] * drho_eff_detai;

    case Jacobian:
      return   _beta[_qp] * _phi[_j][_qp] * (2.0 * SumEtai2 * ((rho_i - _rho_eff[_qp]) - _u[_qp] * drho_eff_detai)
              - 4.0 * _u[_qp] * _u[_qp] * (rho_i - _rho_eff[_qp])) / (SumEtai2 * SumEtai2);
  }
  mooseError("Invalid type passed in");
}
