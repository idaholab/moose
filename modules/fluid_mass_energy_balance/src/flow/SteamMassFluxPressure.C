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
#include "SteamMassFluxPressure.h"

template<>
InputParameters validParams<SteamMassFluxPressure>()
{
  InputParameters params = validParams<Diffusion>();
   params.addCoupledVar("enthalpy", "Use CoupledVar pressure here");
  params.addCoupledVar("pressure", "Use CoupledVar pressure here");
  return params;
}

SteamMassFluxPressure::SteamMassFluxPressure(const std::string & name,
                                             InputParameters parameters)
  :Diffusion(name, parameters),
   _h_var(coupled("enthalpy")),
   _grad_p(coupledGradient("pressure")),
   _Dtau_steamDP(getMaterialProperty<Real>("Dtau_steamDP")),
   _Dtau_steamDH(getMaterialProperty<Real>("Dtau_steamDH")),
   _tau_steam(getMaterialProperty<Real>("tau_steam")),
   _darcy_mass_flux_steam(getMaterialProperty<RealGradient>("darcy_mass_flux_steam"))
{}

Real
SteamMassFluxPressure::computeQpResidual()
{
   return -_darcy_mass_flux_steam[_qp]* _grad_test[_i][_qp];
  //return _tau_steam[_qp]*Diffusion::computeQpResidual();
}

Real
SteamMassFluxPressure::computeQpJacobian()
{  // return _tau_steam[_qp]*Diffusion::computeQpJacobian();
    
  
    return _grad_test[_i][_qp]*(_tau_steam[_qp]*_grad_phi[_j][_qp]+_Dtau_steamDP[_qp]*_phi[_j][_qp]*_grad_p[_qp]);
}

Real SteamMassFluxPressure::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar==_h_var)
  {
    return _grad_test[_i][_qp]*_Dtau_steamDH[_qp]*_phi[_j][_qp]*_grad_p[_qp]; 
  }
  else
  {
    return 0.0;
  }
}
