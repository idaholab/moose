/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "DarcyPressure.h"


template<>
InputParameters validParams<DarcyPressure>()
{
  // Start with the parameters from our parent
  InputParameters params = validParams<Diffusion>();

  // Now add any extra parameters this class needs:

  // Add a required parameter.  If this isn't provided in the input file MOOSE will error.
  params.addParam<Real>("permeability", "The permeability (K) of the fluid");

  // Add a parameter with a default value.  This value can be overriden in the input file.
  params.addRequiredParam<Real>("viscosity", 7.98e-4, "The viscosity (mu) of the fluid.  Default is for 30 degrees C.");

  return params;
}


DarcyPressure::DarcyPressure(const std::string & name, InputParameters parameters) :
    Diffusion(name, parameters),

    // Get the parameters from the input file
    _permeability(getParam<Real>("permeability")),
    _viscosity(getParam<Real>("viscosity"))
{
}

DarcyPressure::~DarcyPressure()
{
}

Real
DarcyPressure::computeQpResidual()
{
  // K/mu * grad_u * grad_phi[i]
  return (_permeability/_viscosity) * Diffusion::computeQpResidual();
}

Real
DarcyPressure::computeQpJacobian()
{
  // K/mu * grad_phi[j] * grad_phi[i]
  return (_permeability/_viscosity) * Diffusion::computeQpJacobian();
}
