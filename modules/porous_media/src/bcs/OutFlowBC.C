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

#include "OutFlowBC.h"

template<>
InputParameters validParams<OutFlowBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  return params;
}

OutFlowBC::OutFlowBC(const std::string & name, InputParameters parameters)
  :IntegratedBC(name, parameters),
   _thermal_conductivity(getMaterialProperty<Real>("thermal_conductivity")),
   _specific_heat_water(getMaterialProperty<Real>("specific_heat_water")),
   _darcy_mass_flux_water(getMaterialProperty<RealGradient>("darcy_mass_flux_water"))
{
}

Real
OutFlowBC::computeQpResidual()
{
  Real val = (-_thermal_conductivity[_qp]*_grad_u[_qp]+_darcy_mass_flux_water[_qp]*_specific_heat_water[_qp]*_u[_qp])*_test[_i][_qp]*_normals[_qp];
  return val;
}


Real
OutFlowBC::computeQpJacobian()
{
  Real val = (-_thermal_conductivity[_qp]*_grad_phi[_j][_qp]+_darcy_mass_flux_water[_qp]*_specific_heat_water[_qp]*_phi[_j][_qp])*_test[_i][_qp]*_normals[_qp];
  return val;

}
