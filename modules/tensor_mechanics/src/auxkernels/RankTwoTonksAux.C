#include "RankTwoTonksAux.h"

template<>
InputParameters validParams<RankTwoTonksAux>()
{
  InputParameters params = validParams<AuxKernel>();

  //add stuff here
  params.addRequiredParam<std::string>("rank_two_tensor", "The rank two material tensor name");
  params.addRequiredParam<int>("index_i", "The index i of ij for the tensor to output (1, 2, 3)");
  params.addRequiredParam<int>("index_j", "The index j of ij for the tensor to output (1, 2, 3)");

  return params;
}

RankTwoTonksAux::RankTwoTonksAux(const std::string & name, InputParameters parameters)
    : AuxKernel(name, parameters),
      _tensor(getMaterialProperty<RankTwoTensorTonks>(getParam<std::string>("rank_two_tensor"))),
      _i(getParam<int>("index_i")),
      _j(getParam<int>("index_j"))
{
}

Real
RankTwoTonksAux::computeValue()
{
  return _tensor[_qp].getValue(_i, _j);
}


