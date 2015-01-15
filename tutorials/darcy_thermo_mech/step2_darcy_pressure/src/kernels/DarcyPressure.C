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
  InputParameters params = validParams<Kernel>();

  params.addRequiredParam<Real>("permeability", "The permeability (K) of the fluid");
  params.addRequiredParam<Real>("viscosity", "The viscosity (mu) of the fluid");

  return params;
}


DarcyPressure::DarcyPressure(const std::string & name, InputParameters parameters) :
    Diffusion(name, parameters),
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
  return (_permeability/_viscosity) * Diffusion::computeQpResidual();
}

Real
DarcyPressure::computeQpJacobian()
{
  return (_permeability/_viscosity) * Diffusion::computeQpJacobian();
}
