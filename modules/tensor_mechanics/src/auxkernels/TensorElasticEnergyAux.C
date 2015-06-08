/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorElasticEnergyAux.h"

template<>
InputParameters validParams<TensorElasticEnergyAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("Compute the local elastic energy");
  return params;
}

TensorElasticEnergyAux::TensorElasticEnergyAux(const std::string & name, InputParameters parameters) :
    AuxKernel( name, parameters ),
    _stress( getMaterialProperty<RankTwoTensor>("stress") ),
    _elastic_strain(getMaterialProperty<RankTwoTensor>("elastic_strain"))
{
}

Real
TensorElasticEnergyAux::computeValue()
{
  // return 0.5 * (_stress[_qp] * _elastic_strain[_qp]);
  return 0.5 * _stress[_qp].doubleContraction(_elastic_strain[_qp]);
}
