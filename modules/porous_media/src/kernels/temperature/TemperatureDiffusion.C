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
#include "TemperatureDiffusion.h"

template<>
InputParameters validParams<TemperatureDiffusion>()
{
  InputParameters params = validParams<Diffusion>();
  return params;
}

TemperatureDiffusion::TemperatureDiffusion(const std::string & name, InputParameters parameters)
  :Diffusion(name, parameters),
   _thermal_conductivity(getMaterialProperty<Real>("thermal_conductivity"))
{}

Real
TemperatureDiffusion::computeQpResidual()
{
  return _thermal_conductivity[_qp]*Diffusion::computeQpResidual();

}

Real
TemperatureDiffusion::computeQpJacobian()
{
  return _thermal_conductivity[_qp]*Diffusion::computeQpJacobian();
}



