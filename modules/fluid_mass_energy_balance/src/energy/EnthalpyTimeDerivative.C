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
#include "Water_Steam_EOS.h"

template<>
InputParameters validParams<EnthalpyTimeDerivative>()
{
    InputParameters params = validParams<TimeDerivative>();
    params.addRequiredCoupledVar("density", "Use CoupledAuxDensity here");
    params.addRequiredCoupledVar("temperature", "Use CoupledAuxDensity here");
    params.addRequiredCoupledVar("dTdH_P", "derivative of water density vs temperature");
    params.addRequiredCoupledVar("dTdP_H", "derivative of water density vs temperature");
    params.addRequiredCoupledVar("ddensitydH_P", "derivative of water density vs temperature");
    params.addCoupledVar("pressure","Use coupled pressuer here to calculate the time derivative");
    params.addRequiredCoupledVar("ddensitydp_H","derivative of density to pressure");
    
    //  params.addRequiredCoupledVar("porosity", "Use CoupledAuxPorosity here");
    return params;
}

EnthalpyTimeDerivative::EnthalpyTimeDerivative(const std::string & name,
                                                     InputParameters parameters)
:TimeDerivative(name, parameters),

_density(coupledValue("density")),
_density_old(coupledValueOld("density")),
_temperature(coupledValue("temperature")),
_temperature_old(coupledValueOld("temperature")),

_dTdH_P(coupledValue("dTdH_P")),
_dTdP_H(coupledValue("dTdP_H")), 
_ddensitydH_P(coupledValue("ddensitydH_P")),
_pressure_old(coupledValueOld("pressure")),
 _p_var(coupled("pressure")),
 _ddensitydp_H(coupledValue("ddensitydp_H")),
_porosity (getMaterialProperty<Real>("material_porosity")),
_specific_heat_rock(getMaterialProperty<Real>("specific_heat_rock")),
 _density_rock(getMaterialProperty<Real>("density_rock")),
 _u_old(valueOld())
{}

Real
EnthalpyTimeDerivative::computeQpResidual()
{
  Real _var[13];
  int _name;
  Real _den_mix;
  Real tt;
  Real _den_old;
  Real _temp_old;
  
  if (_t_step==1) {
    
        Water_Steam_EOS::water_steam_prop_ph_(_pressure_old[_qp], _u_old[_qp], tt, 
                                              _var[0], _den_mix, 
                                              _var[1], _var[2], _var[3], _var[4],
                                              _var[5],_var[6], _var[7], _var[8],
                                              _var[9], _var[10], _name, _var[11], _var[12],_var[13]); 
        
        _den_old=_den_mix; 
        _temp_old=tt; 
  //     std::cout << " temp mix: "<<_den_old<<" "<< tt <<"\n" ;   
  } 
    else 
    { _den_old= _density_old[_qp]; 
        _temp_old = _temperature_old[_qp]; 
    } 
    
    //REAL dphirho_dt = ((_porosity[_qp]*_density_water[_qp])-(_porosity_old[_qp]*_density_water_old[_qp]))/_dt;
    // std::cout <<_porosity[_qp]<< "\n";
    /*
     Real tmp1=(((_porosity[_qp]*_density_water[_qp]*_specific_heat_water[_qp])+
     ((1.0-_porosity[_qp])*_density_rock[_qp]*_specific_heat_rock[_qp]))*_u[_qp]-
     ((_porosity[_qp]*_density_water_old[_qp]*_specific_heat_water[_qp])+
     ((1.0-_porosity[_qp])*_density_rock[_qp]*_specific_heat_rock[_qp]))*_u_old[_qp])*_test[_i][_qp]/_dt;
     */
    //   std::cout <<"dwdt: "<< _dwdt[_qp]<<' '<<_density_water[_qp]<< "\n";
    Real tmp1=(_porosity[_qp]*_density[_qp]*_u[_qp]+
              (1.0-_porosity[_qp])*_density_rock[_qp]*_specific_heat_rock[_qp]*_temperature[_qp]-
              _porosity[_qp]*_den_old*_u_old[_qp]-
                (1.0-_porosity[_qp])*_density_rock[_qp]*_specific_heat_rock[_qp]*_temp_old)
              *_test[_i][_qp] /_dt;
    
    //   Std::cout <<_density_water[_qp] << ' '<< _density_water_old[_qp]<< "\n";
    return tmp1;
    
}

Real
EnthalpyTimeDerivative::computeQpJacobian()
{
    /*    
     Real tmp1 = (((_porosity[_qp]*_density_water[_qp]*_specific_heat_water[_qp])+
     ((1.0-_porosity[_qp])*_density_rock[_qp]*_specific_heat_rock[_qp]))*_phi[_j][_qp])*_test[_i][_qp]/_dt;
     Real tmp2 = _porosity[_qp]*_dwdt[_qp]*_specific_heat_water[_qp]*_u[_qp]*_test[_i][_qp]/_dt;
     */
    
    Real tmp1 = (_porosity[_qp]*(_density[_qp]*_phi[_j][_qp]+_ddensitydH_P[_qp]*_u[_qp]*_phi[_j][_qp])+
                (1.0-_porosity[_qp])*_density_rock[_qp]*_specific_heat_rock[_qp]*_dTdH_P[_qp]*_phi[_j][_qp])
                *_test[_i][_qp] /_dt;  
    
    //  std::cout <<tmp1 << ' '<< tmp2 << "\n";    
    
    return tmp1;    
}

Real EnthalpyTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar==_p_var)
  {
    return (_porosity[_qp]*_ddensitydp_H[_qp]*_u[_qp]*_phi[_j][_qp]+
                (1.0-_porosity[_qp])*_density_rock[_qp]*_specific_heat_rock[_qp]*_dTdP_H[_qp]*_phi[_j][_qp])
                *_test[_i][_qp] /_dt; 
  }
  else
  {
    return 0.0;
  }
}
