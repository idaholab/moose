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
  params.addCoupledVar("enthalpy"," Use Coupled enthalpy to calculate the time derivative");
  return params;
}

MassFluxTimeDerivative::MassFluxTimeDerivative(const std::string & name, InputParameters parameters)
    :TimeDerivative(name, parameters),

     _density(getMaterialProperty<Real>("density")),
     _time_old_density(getMaterialProperty<Real>("time_old_density")),
     _ddensitydp_H(getMaterialProperty<Real>("ddensitydp_H")),
     _ddensitydH_P(getMaterialProperty<Real>("ddensitydH_P")),
     _h_var(coupled("enthalpy")),
     _porosity(getMaterialProperty<Real>("porosity")),
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
  return (_porosity[_qp]*_ddensitydp_H[_qp]*_phi[_j][_qp])*_test[_i][_qp]/_dt;
}

Real MassFluxTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar==_h_var)
    return _porosity[_qp]*_ddensitydH_P[_qp]*_phi[_j][_qp]*_test[_i][_qp]/_dt;
  else
    return 0.0;
}
