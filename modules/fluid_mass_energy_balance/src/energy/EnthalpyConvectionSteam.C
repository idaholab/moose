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
#include "EnthalpyConvectionSteam.h"

template<>
InputParameters validParams<EnthalpyConvectionSteam>()
{
  InputParameters params = validParams<Kernel>();
  params.addCoupledVar("pressure","Use CoupledVariable pressure index here");

  return params;
}

EnthalpyConvectionSteam::EnthalpyConvectionSteam(const std::string & name, InputParameters parameters)
    :Kernel(name, parameters),
     _Dtau_steamDH(getMaterialProperty<Real>("Dtau_steamDH")),
     _Dtau_steamDP(getMaterialProperty<Real>("Dtau_steamDP")),
     _darcy_mass_flux_steam(getMaterialProperty<RealGradient>("darcy_mass_flux_steam")),
     _tau_steam(getMaterialProperty<Real>("tau_steam")),
     _enthalpy_steam(getMaterialProperty<Real>("enthalpy_steam")),
     _denthalpy_steamdH_P(getMaterialProperty<Real>("denthalpy_steamdH_P")),
     _denthalpy_steamdP_H(getMaterialProperty<Real>("denthalpy_steamdP_H")),
     _p_var(coupled("pressure")),
     _grad_p(coupledGradient("pressure"))
{}

Real EnthalpyConvectionSteam::computeQpResidual()
{
  //return  _darcy_mass_flux_steam[_qp]*_grad_enthalpy_steam[_qp]*_test[_i][_qp];
  return -_darcy_mass_flux_steam[_qp]*_enthalpy_steam[_qp]*_grad_test[_i][_qp];
}

Real EnthalpyConvectionSteam::computeQpJacobian()
{
  //  return _darcy_mass_flux_steam[_qp]*_denthalpy_steamdH_P[_qp]*_grad_phi[_j][_qp]*_test[_i][_qp];
  /*  return -_grad_test[_i][_qp]*
      ( _darcy_mass_flux_steam[_qp]*_denthalpy_steamdH_P[_qp]*_phi[_j][_qp]
      + _Ddarcy_mass_flux_steamDH[_qp]* _enthalpy_steam[_qp]*_phi[_j][_qp]);
  */
  return _grad_test[_i][_qp]*(_Dtau_steamDH[_qp]*_phi[_j][_qp]*_grad_p[_qp]*_enthalpy_steam[_qp]
                              +_tau_steam[_qp]*_grad_p[_qp]*_denthalpy_steamdH_P[_qp]*_phi[_j][_qp]);
}

Real EnthalpyConvectionSteam::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar==_p_var)
    // return  _grad_test[_i][_qp]*(_tau_steam[_qp]*_grad_phi[_j][_qp]*_enthalpy_steam[_qp]);
    return _grad_test[_i][_qp]*(_Dtau_steamDP[_qp]*_phi[_j][_qp]*_grad_p[_qp]*_enthalpy_steam[_qp]
                                +_tau_steam[_qp]*_grad_phi[_j][_qp]*_enthalpy_steam[_qp]
                                +_tau_steam[_qp]*_grad_p[_qp]*_denthalpy_steamdP_H[_qp]*_phi[_j][_qp]);
  else
    return 0.0;
}
