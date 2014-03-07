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
    params.addCoupledVar("pressure","Use coupled pressuer here to calculate the time derivative");
    return params;
}

EnthalpyTimeDerivative::EnthalpyTimeDerivative(const std::string & name,
                                                     InputParameters parameters):TimeDerivative(name, parameters),
_density(getMaterialProperty<Real>("density")),
_time_old_density(getMaterialProperty<Real>("time_old_density")),

_temperature(getMaterialProperty<Real>("material_temperature")),
_time_old_temperature(getMaterialProperty<Real>("time_old_material_temperature")),

_dTdH_P(getMaterialProperty<Real>("dTdH_P")),
_dTdP_H(getMaterialProperty<Real>("dTdP_H")),
_ddensitydH_P(getMaterialProperty<Real>("ddensitydH_P")),
_ddensitydp_H(getMaterialProperty<Real>("ddensitydp_H")),

_p_var(coupled("pressure")),

_porosity(getMaterialProperty<Real>("porosity")),
_specific_heat_rock(getMaterialProperty<Real>("specific_heat_rock")),
_density_rock(getMaterialProperty<Real>("density_rock")),
_u_old(valueOld())

{}

Real
EnthalpyTimeDerivative::computeQpResidual()
{
  return (((_porosity[_qp] * _density[_qp] * _u[_qp]) + ((1.0-_porosity[_qp]) * _density_rock[_qp] * _specific_heat_rock[_qp] * _temperature[_qp]))
          - ((_porosity[_qp] * _time_old_density[_qp] * _u_old[_qp]) + ((1.0-_porosity[_qp]) * _density_rock[_qp] * _specific_heat_rock[_qp] * _time_old_temperature[_qp])))
    * _test[_i][_qp] /_dt;
  //REAL dphirho_dt = ((_porosity[_qp]*_density_water[_qp])-(_porosity_old[_qp]*_density_water_old[_qp]))/_dt;
  // Moose::out <<_porosity[_qp]<< "\n";
  /*
    Real tmp1=(((_porosity[_qp]*_density_water[_qp]*_specific_heat_water[_qp])+
    ((1.0-_porosity[_qp])*_density_rock[_qp]*_specific_heat_rock[_qp]))*_u[_qp]-
    ((_porosity[_qp]*_density_water_old[_qp]*_specific_heat_water[_qp])+
    ((1.0-_porosity[_qp])*_density_rock[_qp]*_specific_heat_rock[_qp]))*_u_old[_qp])*_test[_i][_qp]/_dt;
  */
}

Real
EnthalpyTimeDerivative::computeQpJacobian()
{
  return (_porosity[_qp] * (_density[_qp] * _phi[_j][_qp] + _ddensitydH_P[_qp] * _u[_qp] * _phi[_j][_qp])
          + (1.0-_porosity[_qp]) * _density_rock[_qp] * _specific_heat_rock[_qp] * _dTdH_P[_qp] * _phi[_j][_qp])
    *_test[_i][_qp] /_dt;
  /*
        Real tmp1 = (((_porosity[_qp]*_density_water[_qp]*_specific_heat_water[_qp])+
        ((1.0-_porosity[_qp])*_density_rock[_qp]*_specific_heat_rock[_qp]))*_phi[_j][_qp])*_test[_i][_qp]/_dt;
        Real tmp2 = _porosity[_qp]*_dwdt[_qp]*_specific_heat_water[_qp]*_u[_qp]*_test[_i][_qp]/_dt;
  */
}

Real EnthalpyTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar==_p_var)
    return  (_porosity[_qp]*_ddensitydp_H[_qp]*_u[_qp]*_phi[_j][_qp]+
             (1.0-_porosity[_qp])*_density_rock[_qp]*_specific_heat_rock[_qp]*_dTdP_H[_qp]*_phi[_j][_qp])
      *_test[_i][_qp] /_dt;
  else
    return 0.0;
}
