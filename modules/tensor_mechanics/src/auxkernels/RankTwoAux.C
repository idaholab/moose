#include "RankTwoAux.h"

template<>
InputParameters validParams<RankTwoAux>()
{
  InputParameters params = validParams<AuxKernel>();

  //add stuff here
  params.addRequiredParam<std::string>("rank_two_tensor", "The rank two material tensor name");
  params.addRequiredParam<int>("index_i", "The index i of ij for the tensor to output (1, 2, 3)");
  params.addRequiredParam<int>("index_j", "The index j of ij for the tensor to output (1, 2, 3)");

  return params;
}

RankTwoAux::RankTwoAux(const std::string & name, InputParameters parameters)
    : AuxKernel(name, parameters),
      _tensor(getMaterialProperty<RankTwoTensor>(getParam<std::string>("rank_two_tensor"))),
      _i(getParam<int>("index_i")),
      _j(getParam<int>("index_j"))
{
}

Real
RankTwoAux::computeValue()
{
  return _tensor[_qp].getValue(_i, _j);
}


