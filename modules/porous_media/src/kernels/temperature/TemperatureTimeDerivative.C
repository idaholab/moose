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

#include "TemperatureTimeDerivative.h"
#include "Material.h"

template<>
InputParameters validParams<TemperatureTimeDerivative>()
{
  InputParameters params = validParams<TimeDerivative>();
    params.addParam<bool>("has_chem_reactions", false, "flag if chemical reactions are present");
    return params;
}

TemperatureTimeDerivative::TemperatureTimeDerivative(const std::string & name,
                                                     InputParameters parameters)
  :TimeDerivative(name, parameters),
   _density_water(getMaterialProperty<Real>("density_water")),
   _density_water_old(getMaterialProperty<Real>("time_old_density_water")),

   _has_chem_reactions(getParam<bool>("has_chem_reactions")),
   _porosity(getMaterialProperty<Real>("porosity")),
   _porosity_old(_has_chem_reactions ? &getMaterialPropertyOld<Real>("porosity") : &getMaterialProperty<Real>("porosity")),

   _specific_heat_water(getMaterialProperty<Real>("specific_heat_water")),
   _specific_heat_rock(getMaterialProperty<Real>("specific_heat_rock")),
   _density_rock(getMaterialProperty<Real>("density_rock")),
   _u_old(valueOld())
{}

Real
TemperatureTimeDerivative::computeQpResidual()
{  
    
  Real tmp1=(((_porosity[_qp]*_density_water[_qp]*_specific_heat_water[_qp]) 
              + ((1.0-_porosity[_qp])*_density_rock[_qp]*_specific_heat_rock[_qp]))*_u[_qp]
             - (((*_porosity_old)[_qp]*_density_water_old[_qp]*_specific_heat_water[_qp])
                + ((1.0-(*_porosity_old)[_qp])*_density_rock[_qp]*_specific_heat_rock[_qp]))*_u_old[_qp])
    *_test[_i][_qp]/_dt;
  
    
    
  return tmp1;
}

Real
TemperatureTimeDerivative::computeQpJacobian()
{
/*    
  Real tmp1 = (((_porosity[_qp]*_density_water[_qp]*_specific_heat_water[_qp])+
       ((1.0-_porosity[_qp])*_density_rock[_qp]*_specific_heat_rock[_qp]))*_phi[_j][_qp])*_test[_i][_qp]/_dt;
  Real tmp2 = _porosity[_qp]*_dwdt[_qp]*_specific_heat_water[_qp]*_u[_qp]*_test[_i][_qp]/_dt;
*/
  Real tmp1 = (((_porosity[_qp]*_density_water[_qp]*_specific_heat_water[_qp])+
               ((1.0-_porosity[_qp])*_density_rock[_qp]*_specific_heat_rock[_qp]))*
               _phi[_j][_qp])*_test[_i][_qp]/_dt;  
  return tmp1;
}
