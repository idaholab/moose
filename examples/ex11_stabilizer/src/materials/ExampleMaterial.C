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
  return params;
}

ExampleMaterial::ExampleMaterial(const std::string & name,
                                 MooseSystem & moose_system,
                                 InputParameters parameters)
  :Material(name, moose_system, parameters),
   _input_diffusivity(getParam<Real>("diffusivity")),
   _diffusivity(declareProperty<Real>("diffusivity"))
{}

void
ExampleMaterial::computeProperties()
{
  for(unsigned int qp=0; qp<_n_qpoints; qp++)
  {
    _diffusivity[qp] = _input_diffusivity;
  }
}
