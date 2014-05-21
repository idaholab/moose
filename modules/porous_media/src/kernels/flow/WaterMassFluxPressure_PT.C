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
#include "WaterMassFluxPressure_PT.h"

template<>
InputParameters validParams<WaterMassFluxPressure_PT>()
{
  InputParameters params = validParams<Diffusion>();
  return params;
}

WaterMassFluxPressure_PT::WaterMassFluxPressure_PT(const std::string & name,
                                             InputParameters parameters)
  :Diffusion(name, parameters),
   _tau_water(getMaterialProperty<Real>("tau_water"))
{}

Real
WaterMassFluxPressure_PT::computeQpResidual()
{
 return _tau_water[_qp]*Diffusion::computeQpResidual();
}

Real
WaterMassFluxPressure_PT::computeQpJacobian()
{
  return _tau_water[_qp]*Diffusion::computeQpJacobian();
}

