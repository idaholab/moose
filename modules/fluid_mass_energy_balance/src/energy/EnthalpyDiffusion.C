/****************************************************************/
/*             DO NOT MODIFY OR REMOVE THIS HEADER              */
/*          FALCON - Fracturing And Liquid CONvection           */
/*                                                              */
/*       (c) pending 2012 Battelle Energy Alliance, LLC         */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/


#include "Material.h"
#include "EnthalpyDiffusion.h"

template<>
InputParameters validParams<EnthalpyDiffusion>()
{
  InputParameters params = validParams<Diffusion>();
  params.addCoupledVar("pressure", "TODO: add description");
  params.addCoupledVar("temperature", "TODO: add description");

  return params;
}

EnthalpyDiffusion::EnthalpyDiffusion(const std::string & name, InputParameters parameters)
    :Diffusion(name, parameters),

     _grad_T(coupledGradient("temperature")),
     _p_var(coupled("pressure")),
     _thermal_conductivity(getMaterialProperty<Real>("thermal_conductivity")),
     _dTdP_H(getMaterialProperty<Real>("dTdP_H")),
     _dTdH_P(getMaterialProperty<Real>("dTdH_P"))
{}

Real
EnthalpyDiffusion::computeQpResidual()
{
  //  return _thermal_conductivity[_qp]*((_dTbydP_H[_qp]*_grad_p[_qp]*_grad_test[_i][_qp])+(_dTbydH_P[_qp]*Diffusion::computeQpResidual()));
  return  _thermal_conductivity[_qp]*(_grad_T[_qp]*_grad_test[_i][_qp]);
  // return _thermal_conductivity[_qp]* _dTdH_P[_qp] * Diffusion::computeQpResidual();
}

Real
EnthalpyDiffusion::computeQpJacobian()
{
  //return 0;
  return _thermal_conductivity[_qp]*_dTdH_P[_qp]*_phi[_j][_qp] *  Diffusion::computeQpJacobian();
  // return _thermal_conductivity[_qp]*_dTdH_P[_qp]*_grad_phi[_j][_qp];
}

Real EnthalpyDiffusion::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar==_p_var)
    return _thermal_conductivity[_qp]*_dTdP_H[_qp]*_phi[_j][_qp] *  Diffusion::computeQpJacobian();
  // return _thermal_conductivity[_qp]*_dTdP_H[_qp]*_grad_phi[_j][_qp];
  else
    return 0.0;
}
