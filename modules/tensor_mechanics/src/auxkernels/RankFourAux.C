#include "RankFourAux.h"

template<>
InputParameters validParams<RankFourAux>()
{
  InputParameters params = validParams<AuxKernel>();

  //add stuff here
  params.addRequiredParam<std::string>("rank_four_tensor", "The rank four material tensor name");
  params.addRequiredRangeCheckedParam<unsigned int>("index_i", "index_i >= 1 & index_i <= 3", "The index i of ijkl for the tensor to output (1, 2, 3)");
  params.addRequiredRangeCheckedParam<unsigned int>("index_j", "index_j >= 1 & index_j <= 3", "The index j of ijkl for the tensor to output (1, 2, 3)");
  params.addRequiredRangeCheckedParam<unsigned int>("index_k", "index_k >= 1 & index_k <= 3", "The index k of ijkl for the tensor to output (1, 2, 3)");
  params.addRequiredRangeCheckedParam<unsigned int>("index_l", "index_l >= 1 & index_l <= 3", "The index l of ijkl for the tensor to output (1, 2, 3)");

  return params;
}

RankFourAux::RankFourAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _tensor(getMaterialProperty<ElasticityTensorR4>(getParam<std::string>("rank_four_tensor"))),
    _i(getParam<unsigned int>("index_i")),
    _j(getParam<unsigned int>("index_j")),
    _k(getParam<unsigned int>("index_k")),
    _l(getParam<unsigned int>("index_l"))
{
}

Real
RankFourAux::computeValue()
{
  return _tensor[_qp].getValue(_i, _j, _k, _l);
}
