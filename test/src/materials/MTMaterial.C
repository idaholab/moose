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

#include "MTMaterial.h"

template<>
InputParameters validParams<MTMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addParam<Real>("value", 1.0, "lift");
  return params;
}

MTMaterial::MTMaterial(const std::string & name, InputParameters parameters)
  : Material(name, parameters),
    _mat_prop(declareProperty<Real>("matp")),
    _value(getParam<Real>("value"))
{
}

void
MTMaterial::computeProperties()
{
  for (unsigned int qp = 0; qp < _n_qpoints; qp++)
    _mat_prop[qp] = _q_point[qp](0) + _value;              // x + value
}
