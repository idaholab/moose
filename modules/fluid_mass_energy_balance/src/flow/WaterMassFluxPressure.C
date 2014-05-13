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
#include "WaterMassFluxPressure.h"

template<>
InputParameters validParams<WaterMassFluxPressure>()
{
  InputParameters params = validParams<Diffusion>();
  params.addCoupledVar("enthalpy", "Use CoupledVar pressure here");
  //params.addCoupledVar("pressure", "Use CoupledVar pressure here");
  return params;
}

WaterMassFluxPressure::WaterMassFluxPressure(const std::string & name,
                                             InputParameters parameters)
    :Diffusion(name, parameters),
     _h_var(coupled("enthalpy")),
     //_grad_p(coupledGradient("pressure")),
     _grad_p(gradient()),
     _Dtau_waterDP(getMaterialProperty<Real>("Dtau_waterDP")),
     _Dtau_waterDH(getMaterialProperty<Real>("Dtau_waterDH")),
     _tau_water(getMaterialProperty<Real>("tau_water")),
     _darcy_mass_flux_water(getMaterialProperty<RealGradient>("darcy_mass_flux_water"))
{}

Real
WaterMassFluxPressure::computeQpResidual()
{ return -_darcy_mass_flux_water[_qp]* _grad_test[_i][_qp];
  // return _tau_water[_qp]*Diffusion::computeQpResidual();
}

Real
WaterMassFluxPressure::computeQpJacobian()
{ // return _tau_water[_qp]*Diffusion::computeQpJacobian();
  // return _tau_water[_qp]*Diffusion::computeQpJacobian()+Diffusion::computeQpResidual()*_Dtau_waterDH[_qp]*_phi[_j][_qp];
  return _grad_test[_i][_qp]*(_tau_water[_qp]*_grad_phi[_j][_qp]+_Dtau_waterDP[_qp]*_phi[_j][_qp]*_grad_p[_qp]);
}



Real WaterMassFluxPressure::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar==_h_var)
    return _grad_test[_i][_qp]*_Dtau_waterDH[_qp]*_phi[_j][_qp]*_grad_p[_qp];
  else
    return 0.0;
}
