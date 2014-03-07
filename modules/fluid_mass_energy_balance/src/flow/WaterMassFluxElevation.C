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
#include "WaterMassFluxElevation.h"

template<>
InputParameters validParams<WaterMassFluxElevation>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

WaterMassFluxElevation::WaterMassFluxElevation(const std::string & name,
                                             InputParameters parameters)
  :Kernel(name, parameters),
   _density_water(getMaterialProperty<Real>("density_water")),
   _tau_water(getMaterialProperty<Real>("tau_water")),
   _gravity(getMaterialProperty<Real>("gravity")),
   _gravity_vector(getMaterialProperty<RealVectorValue>("gravity_vector"))
{}

Real
WaterMassFluxElevation::computeQpResidual()
{
  //return _tau_water[_qp]*_density_water[_qp]*_gravity[_qp]*_gravity_vector[_qp]*_test[_i][_qp];

  return _tau_water[_qp]*_density_water[_qp]*_gravity[_qp]*(_gravity_vector[_qp]*_grad_test[_i][_qp]);


}

Real
WaterMassFluxElevation::computeQpJacobian()
{
  return 0;
}

