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

  // No parameters are necessary here because we're going to get
  // permeability and viscosity from the Material
  // so we just return params...

  return params;
}


DarcyPressure::DarcyPressure(const std::string & name, InputParameters parameters) :
    Diffusion(name, parameters),

    // Get the permeability and viscosity from the Material system
    // This returns a MaterialProperty<Real> reference that we store
    // in the class and then index into in computeQpResidual/Jacobian....
    _permeability(getMaterialProperty<Real>("permeability")),
    _viscosity(getMaterialProperty<Real>("viscosity"))
{
}

DarcyPressure::~DarcyPressure()
{
}

Real
DarcyPressure::computeQpResidual()
{
  // Use the MaterialProperty references we stored earlier
  return (_permeability[_qp]/_viscosity[_qp]) * Diffusion::computeQpResidual();
}

Real
DarcyPressure::computeQpJacobian()
{
  // Use the MaterialProperty references we stored earlier
  return (_permeability[_qp]/_viscosity[_qp]) * Diffusion::computeQpJacobian();
}
