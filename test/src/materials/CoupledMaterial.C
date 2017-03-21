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
#include "CoupledMaterial.h"

template <>
InputParameters
validParams<CoupledMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<MaterialPropertyName>("mat_prop",
                                                "Name of the property this material defines");
  params.addRequiredParam<MaterialPropertyName>(
      "coupled_mat_prop", "Name of the property to couple into this material");
  params.addParam<bool>(
      "use_old_prop",
      false,
      "Boolean indicating whether to use the old coupled property instead of the current property");
  return params;
}

CoupledMaterial::CoupledMaterial(const InputParameters & parameters)
  : Material(parameters),
    _mat_prop_name(getParam<MaterialPropertyName>("mat_prop")),
    _mat_prop(declareProperty<Real>(_mat_prop_name)),
    _coupled_mat_prop(getParam<bool>("use_old_prop")
                          ? getMaterialPropertyOld<Real>("coupled_mat_prop")
                          : getMaterialProperty<Real>("coupled_mat_prop"))
{
}

void
CoupledMaterial::computeQpProperties()
{
  _mat_prop[_qp] =
      4.0 / _coupled_mat_prop[_qp]; // This will produce a NaN if evaluated out of order
}
