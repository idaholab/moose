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
#include "FluidFluxPressure.h"

template<>
InputParameters validParams<FluidFluxPressure>()
{
    InputParameters params = validParams<Diffusion>();
    return params;
}

FluidFluxPressure::FluidFluxPressure(const std::string & name,
                                                   InputParameters parameters)
:Diffusion(name, parameters),
_tau_water(getMaterialProperty<Real>("tau_water")),
_darcy_mass_flux_water(getMaterialProperty<RealGradient>("darcy_mass_flux_water"))
{}

Real
FluidFluxPressure::computeQpResidual()
{
//  std::cout << _tau_water[_qp] << "\n";
    return _tau_water[_qp]*Diffusion::computeQpResidual();
    //  return -_darcy_mass_flux_water[_qp]*_grad_test[_i][_qp]; //_tau_water[_qp]*Diffusion::computeQpResidual();
}

Real
FluidFluxPressure::computeQpJacobian()
{
    return _tau_water[_qp]*Diffusion::computeQpJacobian();
}

