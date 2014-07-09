#include "RankTwoAux.h"

template<>
InputParameters validParams<RankTwoAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<std::string>("rank_two_tensor", "The rank two material tensor name");
  params.addRequiredRangeCheckedParam<unsigned int>("index_i", "index_i >= 1 & index_i <= 3", "The index i of ij for the tensor to output (1, 2, 3)");
  params.addRequiredRangeCheckedParam<unsigned int>("index_j", "index_j >= 1 & index_j <= 3", "The index j of ij for the tensor to output (1, 2, 3)");
  return params;
}

RankTwoAux::RankTwoAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _tensor(getMaterialProperty<RankTwoTensor>(getParam<std::string>("rank_two_tensor"))),
    _i(getParam<unsigned int>("index_i")),
    _j(getParam<unsigned int>("index_j"))
{
}

Real
RankTwoAux::computeValue()
{
  return _tensor[_qp].getValue(_i, _j);
}
