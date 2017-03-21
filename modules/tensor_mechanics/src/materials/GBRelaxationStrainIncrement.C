/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GBRelaxationStrainIncrement.h"
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<GBRelaxationStrainIncrement>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Compute strain increment based on lattice relaxation at GB");
  params.addParam<MaterialPropertyName>("prefactor_name", "Name of perfactor property");
  params.addParam<MaterialPropertyName>("gb_normal_name", "Name of GB normal property");
  params.addParam<MaterialPropertyName>("property_name",
                                        "Name of GB relaxation strain increment property");
  return params;
}

GBRelaxationStrainIncrement::GBRelaxationStrainIncrement(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _prefactor(getMaterialProperty<Real>("prefactor_name")),
    _gb_normal_tensor(getMaterialProperty<RankTwoTensor>("gb_normal_name")),
    _strain_increment(
        declareProperty<RankTwoTensor>(getParam<MaterialPropertyName>("property_name")))
{
}

void
GBRelaxationStrainIncrement::initQpStatefulProperties()
{
  _strain_increment[_qp].zero();
}

void
GBRelaxationStrainIncrement::computeQpProperties()
{
  _strain_increment[_qp] = _prefactor[_qp] * _dt * _gb_normal_tensor[_qp];
}
