/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ElasticEnergyAux.h"

template <>
InputParameters
validParams<ElasticEnergyAux>()
{
  InputParameters params = validParams<AuxKernel>();
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
