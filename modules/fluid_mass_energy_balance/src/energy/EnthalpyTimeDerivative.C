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

#include "EnthalpyTimeDerivative.h"
#include "Material.h"
#include "WaterSteamEOS.h"

template<>
InputParameters validParams<EnthalpyTimeDerivative>()
{
    InputParameters params = validParams<TimeDerivative>();
    //params.addCoupledVar("temperature", "Use CoupledAuxTemperature here");
    params.addCoupledVar("pressure","Use coupled pressuer here to calculate the time derivative");
    params.addRequiredParam<UserObjectName>("water_steam_properties", "EOS functions, calculate water and steam properties");
    //params.addRequiredCoupledVar("density", "Use CoupledAuxDensity here");                        //reomved by kat
    //params.addRequiredCoupledVar("dTdH_P", "derivative of water density vs temperature");         //removed by kat
    //params.addRequiredCoupledVar("dTdP_H", "derivative of water density vs temperature");         //removed by kat
    //params.addRequiredCoupledVar("ddensitydH_P", "derivative of water density vs temperature");   //removed by kat
    //params.addRequiredCoupledVar("ddensitydp_H","derivative of density to pressure");             //removed by kat
    //params.addRequiredCoupledVar("porosity", "Use CoupledAuxPorosity here");
    return params;
}

EnthalpyTimeDerivative::EnthalpyTimeDerivative(const std::string & name,
                                                     InputParameters parameters):TimeDerivative(name, parameters),

_water_steam_properties(getUserObject<WaterSteamEOS>("water_steam_properties")),

//_density(coupledValue("density")),                    //removed by Kat
//_density_old(coupledValueOld("density")),             //removed by Kat

_density(getMaterialProperty<Real>("density")),                         //added by Kat
_time_old_density(getMaterialProperty<Real>("time_old_density")),       //added by Kat

//_temperature(coupledValue("temperature")),
//_temperature_old(coupledValueOld("temperature")),
_temperature(getMaterialProperty<Real>("material_temperature")),
_time_old_temperature(getMaterialProperty<Real>("time_old_material_temperature")),    //added by kat

//_dTdH_P(coupledValue("dTdH_P")),                      //removed by kat
//_dTdP_H(coupledValue("dTdP_H")),                      //removed by kat
//_ddensitydH_P(coupledValue("ddensitydH_P")),          //romoved by kat
//_ddensitydp_H(coupledValue("ddensitydp_H")),          //removed by kat
_dTdH_P(getMaterialProperty<Real>("dTdH_P")),                           //added by kat
_dTdP_H(getMaterialProperty<Real>("dTdP_H")),                           //added by kat
_ddensitydH_P(getMaterialProperty<Real>("ddensitydH_P")),               //added by kat
_ddensitydp_H(getMaterialProperty<Real>("ddensitydp_H")),               //added by kat

//_pressure_old(coupledValueOld("pressure")),
_p_var(coupled("pressure")),

//_ddensitydp_H(coupledValue("ddensitydp_H")),          //removed by kat
_porosity (getMaterialProperty<Real>("material_porosity")),
_specific_heat_rock(getMaterialProperty<Real>("specific_heat_rock")),
_density_rock(getMaterialProperty<Real>("density_rock")),
_u_old(valueOld())

{}

Real
EnthalpyTimeDerivative::computeQpResidual()
{   
    
    /*Real tmp1 = (((_porosity[_qp] * _density[_qp] * _u[_qp]) + ((1.0-_porosity[_qp]) * _density_rock[_qp] * _specific_heat_rock[_qp] * _temperature[_qp])) 
                 - ((_porosity[_qp] * _time_old_density[_qp] * _u_old[_qp]) + ((1.0-_porosity[_qp]) * _density_rock[_qp] * _specific_heat_rock[_qp] * _time_old_temperature[_qp])))
                * _test[_i][_qp] /_dt;
     */
    

    Real tmp1 = (((_porosity[_qp] * _density[_qp] * _u[_qp]) + ((1.0-_porosity[_qp]) * _density_rock[_qp] * _specific_heat_rock[_qp] * _temperature[_qp])) 
               - ((_porosity[_qp] * _time_old_density[_qp] * _u_old[_qp]) + ((1.0-_porosity[_qp]) * _density_rock[_qp] * _specific_heat_rock[_qp] * _time_old_temperature[_qp])))
                * _test[_i][_qp] /_dt;
    
    //Real tmp1 = (((_porosity[_qp] * _density[_qp]) + ((1.0-_porosity[_qp]) * _density_rock[_qp] * _specific_heat_rock[_qp] * _temperature[_qp])) 
      //           - ((_porosity[_qp] * _time_old_density[_qp]) + ((1.0-_porosity[_qp]) * _density_rock[_qp] * _specific_heat_rock[_qp] * _time_old_temperature[_qp])))*TimeDerivative:://computeQpResidual();    
    
    
    //REAL dphirho_dt = ((_porosity[_qp]*_density_water[_qp])-(_porosity_old[_qp]*_density_water_old[_qp]))/_dt;
    // std::cout <<_porosity[_qp]<< "\n";
    /*
    Real tmp1=(((_porosity[_qp]*_density_water[_qp]*_specific_heat_water[_qp])+
                ((1.0-_porosity[_qp])*_density_rock[_qp]*_specific_heat_rock[_qp]))*_u[_qp]-
               ((_porosity[_qp]*_density_water_old[_qp]*_specific_heat_water[_qp])+
                ((1.0-_porosity[_qp])*_density_rock[_qp]*_specific_heat_rock[_qp]))*_u_old[_qp])*_test[_i][_qp]/_dt;    
    */
             
    return tmp1;
    
}

Real
EnthalpyTimeDerivative::computeQpJacobian()
{
    
    Real tmp1 = (_porosity[_qp] * (_density[_qp] * _phi[_j][_qp] + _ddensitydH_P[_qp] * _u[_qp] * _phi[_j][_qp])
                 + (1.0-_porosity[_qp]) * _density_rock[_qp] * _specific_heat_rock[_qp] * _dTdH_P[_qp] * _phi[_j][_qp])
                *_test[_i][_qp] /_dt;  
    
    //Real tmp1 = (_porosity[_qp] * (_density[_qp] + _ddensitydH_P[_qp])
      //          + (1.0-_porosity[_qp]) * _density_rock[_qp] * _specific_heat_rock[_qp] * _dTdH_P[_qp])
        //        *TimeDerivative::computeQpJacobian();     
    
    
    /*    
     Real tmp1 = (((_porosity[_qp]*_density_water[_qp]*_specific_heat_water[_qp])+
     ((1.0-_porosity[_qp])*_density_rock[_qp]*_specific_heat_rock[_qp]))*_phi[_j][_qp])*_test[_i][_qp]/_dt;
     Real tmp2 = _porosity[_qp]*_dwdt[_qp]*_specific_heat_water[_qp]*_u[_qp]*_test[_i][_qp]/_dt;
     */
         
    return tmp1;    
}

Real EnthalpyTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar==_p_var)
  {
    return  (_porosity[_qp]*_ddensitydp_H[_qp]*_u[_qp]*_phi[_j][_qp]+
            (1.0-_porosity[_qp])*_density_rock[_qp]*_specific_heat_rock[_qp]*_dTdP_H[_qp]*_phi[_j][_qp])
               *_test[_i][_qp] /_dt; 

  }
  else
  {
    return 0.0;
  }
}
