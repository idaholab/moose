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
#include "WaterMassFluxPressurePT.h"

template<>
InputParameters validParams<WaterMassFluxPressurePT>()
{
  InputParameters params = validParams<Diffusion>();
  return params;
}

WaterMassFluxPressurePT::WaterMassFluxPressurePT(const std::string & name,
                                             InputParameters parameters)
  :Diffusion(name, parameters),
   _tau_water(getMaterialProperty<Real>("tau_water"))
{}

Real
WaterMassFluxPressurePT::computeQpResidual()
{
 return _tau_water[_qp]*Diffusion::computeQpResidual();
}

Real
WaterMassFluxPressurePT::computeQpJacobian()
{
  return _tau_water[_qp]*Diffusion::computeQpJacobian();
}

