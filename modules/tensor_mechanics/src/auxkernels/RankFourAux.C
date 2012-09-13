#include "RankFourAux.h"

template<>
InputParameters validParams<RankFourAux>()
{
  InputParameters params = validParams<AuxKernel>();

  //add stuff here
  params.addRequiredParam<std::string>("rank_four_tensor", "The rank four material tensor name");
  params.addRequiredParam<int>("index_i", "The index i of ijkl for the tensor to output (1, 2, 3)");
  params.addRequiredParam<int>("index_j", "The index j of ijkl for the tensor to output (1, 2, 3)");
  params.addRequiredParam<int>("index_k", "The index k of ijkl for the tensor to output (1, 2, 3)");
  params.addRequiredParam<int>("index_l", "The index l of ijkl for the tensor to output (1, 2, 3)");

  return params;
}

RankFourAux::RankFourAux(const std::string & name, InputParameters parameters)
    : AuxKernel(name, parameters),
      _tensor(getMaterialProperty<ElasticityTensorR4>(getParam<std::string>("rank_four_tensor"))),
      _i(getParam<int>("index_i")),
      _j(getParam<int>("index_j")),
      _k(getParam<int>("index_k")),
      _l(getParam<int>("index_l"))
{
}

Real
RankFourAux::computeValue()
{
  return _tensor[_qp].getValue(_i, _j, _k, _l);
}


