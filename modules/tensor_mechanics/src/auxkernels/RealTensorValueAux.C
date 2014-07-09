#include "RealTensorValueAux.h"

template<>
InputParameters validParams<RealTensorValueAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<std::string>("tensor", "The material tensor name");
  params.addRequiredParam<unsigned int>("index_i", "The index i of ij for the tensor to output (0,1,2)");
  params.addRequiredParam<unsigned int>("index_j", "The index j of ij for the tensor to output (0,1,2)");
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
