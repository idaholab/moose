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
  params.addParam<Real>("diffusivity", 1.0, "The Diffusivity");
  params.addParam<Real>("time_coefficient", 1.0, "Time Coefficient");
  return params;
}

ExampleMaterial::ExampleMaterial(const std::string & name,
                                 InputParameters parameters) :
    Material(name, parameters),

    // Get a parameter value for the diffusivity
    _input_diffusivity(getParam<Real>("diffusivity")),

    // Get a parameter value for the time_coefficient
    _input_time_coefficient(getParam<Real>("time_coefficient")),

    // Declare that this material is going to have a Real
    // valued property named "diffusivity" that Kernels can use.
    _diffusivity(declareProperty<Real>("diffusivity")),

    // Declare that this material is going to have a Real
    // valued property named "time_coefficient" that Kernels can use.
    _time_coefficient(declareProperty<Real>("time_coefficient"))
{}

void
ExampleMaterial::computeQpProperties()
{
  _diffusivity[_qp] = _input_diffusivity;
  _time_coefficient[_qp] = _input_time_coefficient;
}
