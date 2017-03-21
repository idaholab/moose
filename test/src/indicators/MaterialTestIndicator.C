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

#include "MaterialTestIndicator.h"

#include "Assembly.h"
#include "SystemBase.h"

template <>
InputParameters
validParams<MaterialTestIndicator>()
{
  InputParameters params = validParams<Indicator>();
  params += validParams<MaterialPropertyInterface>();
  params.addParam<MaterialPropertyName>(
      "property", "The name of the material property to use for an indicator.");
  return params;
}

MaterialTestIndicator::MaterialTestIndicator(const InputParameters & parameters)
  : Indicator(parameters),
    _property(getMaterialProperty<Real>("property")),
    _qrule(_assembly.qRule()),
    _indicator_var(_sys.getVariable(_tid, name()))
{
}

void
MaterialTestIndicator::computeIndicator()
{
  Real min = std::numeric_limits<Real>::max();
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    min = std::min(min, _property[qp]);
  _indicator_var.setNodalValue(min);
}
