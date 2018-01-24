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
#include "QpMaterial.h"

template <>
InputParameters
validParams<QpMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<std::string>("property_name",
                                       "The desired name for the Material Property.");
  return params;
}

QpMaterial::QpMaterial(const InputParameters & parameters)
  : Material(parameters),
    _prop_name(getParam<std::string>("property_name")),
    _mat_prop(declareProperty<Real>(_prop_name))
{
}

void
QpMaterial::computeQpProperties()
{
  _mat_prop[_qp] = _qp;
}
