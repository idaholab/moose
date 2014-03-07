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

#include "EnthalpyImplicitEuler.h"
#include "Material.h"

template<>
InputParameters validParams<EnthalpyImplicitEuler>()
{
  InputParameters params = validParams<TimeDerivative>();
  params.addCoupledVar("temperature", "TODO: add description");
  return params;
}

EnthalpyImplicitEuler::EnthalpyImplicitEuler(const std::string & name, InputParameters parameters)
  :TimeDerivative(name, parameters),

   _temperature(coupledValue("temperature")),
   _temperature_old(coupledValue("temperature_old")),
   _density(getMaterialProperty<Real>("density")),
   _density_old(getMaterialProperty<Real>("time_old_density")),

   _porosity(getMaterialProperty<Real>("porosity")),
   _density_rock(getMaterialProperty<Real>("density_rock")),
   _u_old(valueOld())


{}

Real
EnthalpyImplicitEuler::computeQpResidual()
{

  Real Heat = (_porosity[_qp]* _density[_qp]*_u[_qp])+((1-_porosity[_qp])*_density_rock[_qp]*879*_temperature[_qp]);
  Real Heat_old = (_porosity[_qp]* _density_old[_qp]*_u_old[_qp])+((1-_porosity[_qp])*_density_rock[_qp]*879*_temperature_old[_qp]);

   return _test[_i][_qp]*(Heat-Heat_old)/_dt;
}

Real
EnthalpyImplicitEuler::computeQpJacobian()
{

   return (_porosity[_qp]* _density[_qp]*TimeDerivative::computeQpJacobian());

}
