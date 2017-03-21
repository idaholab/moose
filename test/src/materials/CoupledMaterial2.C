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
#include "CoupledMaterial2.h"

template <>
InputParameters
validParams<CoupledMaterial2>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<MaterialPropertyName>("mat_prop",
                                                "Name of the property this material defines");
  params.addRequiredParam<MaterialPropertyName>(
      "coupled_mat_prop1", "Name of the first property to couple into this material");
  params.addRequiredParam<MaterialPropertyName>(
      "coupled_mat_prop2", "Name of the second property to couple into this material");
  return params;
}

CoupledMaterial2::CoupledMaterial2(const InputParameters & parameters)
  : Material(parameters),
    _mat_prop_name(getParam<MaterialPropertyName>("mat_prop")),
    _mat_prop(declareProperty<Real>(_mat_prop_name)),
    _coupled_mat_prop(getMaterialProperty<Real>("coupled_mat_prop1")),
    _coupled_mat_prop2(getMaterialProperty<Real>("coupled_mat_prop2"))
{
}

void
CoupledMaterial2::computeQpProperties()
{
  _mat_prop[_qp] = 4.0 / _coupled_mat_prop[_qp] /
                   _coupled_mat_prop2[_qp]; // This will produce a NaN if evaluated out of order
}
