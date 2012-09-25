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

#include "MassFluxTimeDerivative.h"
#include "Material.h"
#include "WaterSteamEOS.h"

template<>
InputParameters validParams<MassFluxTimeDerivative>()
{
  InputParameters params = validParams<TimeDerivative>();
   //params.addRequiredCoupledVar("density", "Use Coupled density here to calculate the time derivative");  //removed by kat
   //params.addRequiredCoupledVar("ddensitydp_H", "derivative of water density vs temperature");            //removed by kat
   params.addCoupledVar("enthalpy"," Use Coupled enthalpy to calculate the time derivative");
   //params.addRequiredCoupledVar("ddensitydH_P","derivative of density vs enthalpy");                      //removed by kat
    params.addRequiredParam<UserObjectName>("water_steam_properties", "EOS functions, calculate water and steam properties");
    
    return params;
}

MassFluxTimeDerivative::MassFluxTimeDerivative(const std::string & name, InputParameters parameters)
  :TimeDerivative(name, parameters),

_water_steam_properties(getUserObject<WaterSteamEOS>("water_steam_properties")),
   
//_density(coupledValue("density")),                         //removed by kat
//_density_old(coupledValueOld("density")),                  //removed by kat
//_ddensitydH_P(coupledValue("ddensitydH_P")),               //removed by kat
//_ddensitydp_H(coupledValue("ddensitydp_H")),               //removed by kat
_density(getMaterialProperty<Real>("density")),                     //added by kat
_time_old_density(getMaterialProperty<Real>("time_old_density")),   //added by kat
_ddensitydp_H(getMaterialProperty<Real>("ddensitydp_H")),           //added by kat
_ddensitydH_P(getMaterialProperty<Real>("ddensitydH_P")),           //added by kat

//_enthalpy_old(coupledValueOld("enthalpy")),                   //removed by kat
_h_var(coupled("enthalpy")),
_porosity (getMaterialProperty<Real>("material_porosity")),
//   _porosity(coupledValue("porosity")),
//   _porosity_old(coupledValueOld("porosity"))
_u_old(valueOld())
{}

Real
MassFluxTimeDerivative::computeQpResidual()
{
    
    return (((_porosity[_qp]*_density[_qp])-(_porosity[_qp]*_time_old_density[_qp]))/_dt) * _test[_i][_qp];
}

Real
MassFluxTimeDerivative::computeQpJacobian()
{     
   Real tmp1 = (_porosity[_qp]*_ddensitydp_H[_qp]*_phi[_j][_qp])*_test[_i][_qp]/_dt;  
    return tmp1;
}

Real MassFluxTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar==_h_var)
  {      
    return _porosity[_qp]*_ddensitydH_P[_qp]*_phi[_j][_qp]*_test[_i][_qp]/_dt;
  }
  else
  {
    return 0.0;
  }
}
