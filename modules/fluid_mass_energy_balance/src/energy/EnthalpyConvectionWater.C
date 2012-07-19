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
#include "EnthalpyConvectionWater.h"

template<>
InputParameters validParams<EnthalpyConvectionWater>()
{
    InputParameters params = validParams<Kernel>();
    params.addRequiredCoupledVar("enthalpy_water", "Use CoupledAuxwater enthalpy here");
    params.addRequiredCoupledVar("denthalpy_waterdH_P", "Use CoupledAux dsteamenthalpydh_P here");
    params.addRequiredCoupledVar("denthalpy_waterdP_H"," use coupledAux");
    params.addCoupledVar("pressure", "Use CoupledVar pressure here");
    return params;
}

EnthalpyConvectionWater::EnthalpyConvectionWater(const std::string & name, InputParameters parameters)
:Kernel(name, parameters),
 _Dtau_waterDH(getMaterialProperty<Real>("Dtau_waterDH")),
 _Dtau_waterDP(getMaterialProperty<Real>("Dtau_waterDP")),
_darcy_mass_flux_water(getMaterialProperty<RealGradient>("darcy_mass_flux_water")),
 _tau_water(getMaterialProperty<Real>("tau_water")),
_enthalpy_water(coupledValue("enthalpy_water")),
 _denthalpy_waterdH_P(coupledValue("denthalpy_waterdH_P")),
 _denthalpy_waterdP_H(coupledValue("denthalpy_waterdP_H")),
 _p_var(coupled("pressure")),
 _grad_p(coupledGradient("pressure"))
{}

Real EnthalpyConvectionWater::computeQpResidual()
{
    
    
   // return  _darcy_mass_flux_water[_qp]*_grad_enthalpy_water[_qp]*_test[_i][_qp];
  return  -_darcy_mass_flux_water[_qp]*_enthalpy_water[_qp]*_grad_test[_i][_qp]; 
}

Real EnthalpyConvectionWater::computeQpJacobian()
{
    
    //return _darcy_mass_flux_water[_qp]*_denthalpy_waterdH_P[_qp]*_grad_phi[_j][_qp]*_test[_i][_qp];
  /* return -_grad_test[_i][_qp]*
            (_darcy_mass_flux_water[_qp]*_denthalpy_waterdH_P[_qp]*_phi[_j][_qp]
             +_Ddarcy_mass_flux_waterDH[_qp]*_enthalpy_water[_qp]*_phi[_j][_qp]);
  */
  return _grad_test[_i][_qp]*(_Dtau_waterDH[_qp]*_phi[_j][_qp]*_grad_p[_qp]*_enthalpy_water[_qp]
                              +_tau_water[_qp]*_grad_p[_qp]*_denthalpy_waterdH_P[_qp]*_phi[_j][_qp]);
    
}

Real EnthalpyConvectionWater::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar==_p_var)
  {//return _grad_test[_i][_qp]*(_tau_water[_qp]*_grad_phi[_j][_qp]*_enthalpy_water[_qp]);
    return _grad_test[_i][_qp]*(_Dtau_waterDP[_qp]*_phi[_j][_qp]*_grad_p[_qp]*_enthalpy_water[_qp]
                                +_tau_water[_qp]*_grad_phi[_j][_qp]*_enthalpy_water[_qp]
                               +_tau_water[_qp]*_grad_p[_qp]*_denthalpy_waterdP_H[_qp]*_phi[_j][_qp]);
    
  }
  else
  {return 0.0;
  }
  
}
