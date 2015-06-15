/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "RealTensorValueAux.h"

template<>
InputParameters validParams<RealTensorValueAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("Access a component of a RealTensorValue");
  params.addRequiredParam<std::string>("tensor", "The material tensor name");
  params.addRequiredRangeCheckedParam<unsigned int>("index_i", "index_i >= 0 & index_i <= 2", "The index i of ij for the tensor to output (0, 1, 2)");
  params.addRequiredRangeCheckedParam<unsigned int>("index_j", "index_j >= 0 & index_j <= 2", "The index j of ij for the tensor to output (0, 1, 2)");
  return params;
}

RealTensorValueAux::RealTensorValueAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _tensor(getMaterialProperty<RealTensorValue>(getParam<std::string>("tensor"))),
    _i(getParam<unsigned int>("index_i")),
    _j(getParam<unsigned int>("index_j"))
{
}

Real
RealTensorValueAux::computeValue()
{
  return _tensor[_qp](_i, _j);
}
