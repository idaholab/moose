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

#include "ExampleMaterial.h"

template<>
InputParameters validParams<ExampleMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addParam<Real>("diffusivity_baseline", 1.0, "This number will be added to the value of some_variable");
  params.addCoupledVar("some_variable", "The value of this variable will be added to diffusivity_baseline to determine the diffusivity");
  
  return params;
}

ExampleMaterial::ExampleMaterial(const std::string & name,
                                 MooseSystem & moose_system,
                                 InputParameters parameters)
  :Material(name, moose_system, parameters),
   
   // Get a parameter
   _diffusivity_baseline(getParam<Real>("diffusivity_baseline")),

   // Couple to a variable for a "nonlinear" material property
   _some_variable(coupledValue("some_variable")),

   // Declare that this material is going to have a Real
   // valued property named "diffusivity" that Kernels can use.
   _diffusivity(declareProperty<Real>("diffusivity"))
{}

void
ExampleMaterial::computeQpProperties()
{
  _diffusivity[_qp] = _diffusivity_baseline + _some_variable[_qp];
}
