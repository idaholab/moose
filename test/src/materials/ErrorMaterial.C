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
#include "ErrorMaterial.h"

template<>
InputParameters validParams<ErrorMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Material that throws an error if its computeQpProperties method gets called.");
  return params;
}

ErrorMaterial::ErrorMaterial(const InputParameters & parameters) :
    Material(parameters)
{
}

void
ErrorMaterial::computeQpProperties()
{
  mooseError("Why are you calling me when I don't provide any properties?");
}
