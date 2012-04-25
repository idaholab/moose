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
#include "Water_Steam_EOS.h"
template<>
InputParameters validParams<MassFluxTimeDerivative>()
{
  InputParameters params = validParams<TimeDerivative>();
   params.addRequiredCoupledVar("density", "Use Coupled density here to calculate the time derivative");
   params.addRequiredCoupledVar("ddensitydp_H", "derivative of water density vs temperature");
   params.addCoupledVar("enthalpy"," Use Coupled enthalpy to calculate the time derivative");
   params.addRequiredCoupledVar("ddensitydH_P","derivative of density vs enthalpy");
   
    return params;
}

MassFluxTimeDerivative::MassFluxTimeDerivative(const std::string & name, InputParameters parameters)
  :TimeDerivative(name, parameters),
   
   _density(coupledValue("density")),
   _density_old(coupledValueOld("density")),
   _ddensitydp_H(coupledValue("ddensitydp_H")),
   _enthalpy_old(coupledValueOld("enthalpy")),
   _h_var(coupled("enthalpy")),
   _ddensitydH_P(coupledValue("ddensitydH_P")),
   _porosity (getMaterialProperty<Real>("material_porosity")),
//   _porosity(coupledValue("porosity")),
//   _porosity_old(coupledValueOld("porosity"))
   _u_old(valueOld())
{}

Real
MassFluxTimeDerivative::computeQpResidual()
{
  //std::cout << _density[_qp] << " " << _density_old[_qp] << "\n";
   Real _var[13];
  int _name;
  Real _den_mix;
  Real _den_old;
  Real tt;
  
  if (_t_step==1)  {
      
     Water_Steam_EOS::water_steam_prop_ph_(_u_old[_qp], _enthalpy_old[_qp], tt, 
                                            _var[0], _den_mix, 
                                            _var[1], _var[2], _var[3], _var[4],
                                            _var[5],_var[6], _var[7], _var[8],
                                            _var[9], _var[10], _name, _var[11], _var[12],_var[13]); 
      
       _den_old=_den_mix; 
     // std::cout << " density mix: "<<_enthalpy_old[_qp]<<" "<<_den_mix<<"\n" ; 
    } 
    else 
    { _den_old= _density_old[_qp];} 

   // std::cout << " density old: "<<_u_old[_qp]<<" "<<_den_old<<"\n" ;   
   return (((_porosity[_qp]*_density[_qp])-(_porosity[_qp]*_den_old))/_dt) * _test[_i][_qp];
}

Real
MassFluxTimeDerivative::computeQpJacobian()
{ 
   // std::cout << _ddensitydp_H[_qp]<<"\n" ;
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
