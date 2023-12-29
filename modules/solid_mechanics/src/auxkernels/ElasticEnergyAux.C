//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElasticEnergyAux.h"

registerMooseObject("TensorMechanicsApp", ElasticEnergyAux);

InputParameters
ElasticEnergyAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Compute the local elastic energy");
  params.addParam<std::string>("base_name", "Mechanical property base name");
  return params;
}

ElasticEnergyAux::ElasticEnergyAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _stress(getMaterialProperty<RankTwoTensor>(_base_name + "stress")),
    _elastic_strain(getMaterialProperty<RankTwoTensor>(_base_name + "elastic_strain"))
{
}

Real
ElasticEnergyAux::computeValue()
{
  return 0.5 * _stress[_qp].doubleContraction(_elastic_strain[_qp]);
}
