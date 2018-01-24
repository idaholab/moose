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
#include "IncrementMaterial.h"

template <>
InputParameters
validParams<IncrementMaterial>()
{
  InputParameters params = validParams<GenericConstantMaterial>();
  params.addClassDescription(
      "Material that tracks the number of times computeQpProperties has been called.");
  return params;
}

IncrementMaterial::IncrementMaterial(const InputParameters & parameters)
  : GenericConstantMaterial(parameters), _inc(0), _mat_prop(declareProperty<Real>("mat_prop"))
{
}

void
IncrementMaterial::computeQpProperties()
{
  GenericConstantMaterial::computeQpProperties();
  _inc++;
  _mat_prop[_qp] = _inc;
}
