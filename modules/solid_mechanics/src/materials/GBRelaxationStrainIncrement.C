//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GBRelaxationStrainIncrement.h"
#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", GBRelaxationStrainIncrement);

InputParameters
GBRelaxationStrainIncrement::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute strain increment based on lattice relaxation at GB");
  params.addParam<MaterialPropertyName>("prefactor_name", "Name of prefactor property");
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
