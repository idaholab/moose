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
#include "TemperatureConvection.h"

template<>
InputParameters validParams<TemperatureConvection>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

TemperatureConvection::TemperatureConvection(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _specific_heat_water(getMaterialProperty<Real>("specific_heat_water")),
   _darcy_mass_flux_water(getMaterialProperty<RealGradient>("darcy_mass_flux_water"))
{}

Real TemperatureConvection::computeQpResidual()
{
//  return _specific_heat_water[_qp]*_darcy_mass_flux_water[_qp]*_test[_i][_qp]*_grad_u[_qp];
  return -_specific_heat_water[_qp]*_darcy_mass_flux_water[_qp]*_grad_test[_i][_qp]*_u[_qp];
}

Real TemperatureConvection::computeQpJacobian()
{
//  return _specific_heat_water[_qp]*_darcy_mass_flux_water[_qp]*_test[_i][_qp]*_grad_phi[_j][_qp];
    return -_specific_heat_water[_qp]*_darcy_mass_flux_water[_qp]*_grad_test[_i][_qp]*_phi[_j][_qp];
}

