/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "VectorRankTwoAux.h"

template <>
InputParameters
validParams<VectorRankTwoAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("Access a component in a list of RankTwoTensors");
  params.addRequiredParam<MaterialPropertyName>("rank_two_tensors",
                                                "The rank two material tensors name");
  params.addRequiredParam<unsigned int>("position", "index of the tensor in the list to output");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "index_i",
      "index_i >= 0 & index_i <= 2",
      "The index i of ij for the tensor to output (0, 1, 2)");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "index_j",
      "index_j >= 0 & index_j <= 2",
      "The index j of ij for the tensor to output (0, 1, 2)");
  params.addParam<unsigned int>("selected_qp", "Evaluate the tensor at this specific quadpoint");
  params.addParamNamesToGroup("selected_qp", "Advanced");

  return params;
}

VectorRankTwoAux::VectorRankTwoAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _tensors(getMaterialProperty<std::vector<RankTwoTensor>>("rank_two_tensors")),
    _position(getParam<unsigned int>("position")),
    _i(getParam<unsigned int>("index_i")),
    _j(getParam<unsigned int>("index_j")),
    _has_selected_qp(isParamValid("selected_qp")),
    _selected_qp(_has_selected_qp ? getParam<unsigned int>("selected_qp") : 0)
{
}

Real
VectorRankTwoAux::computeValue()
{
  unsigned int qp = _qp;
  if (_has_selected_qp)
  {
    if (_selected_qp >= _q_point.size())
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      mooseError("VectorRankFourAux.  selected_qp specified as ",
                 _selected_qp,
                 " but there are only ",
                 _q_point.size(),
                 " quadpoints in the element");
    }
    qp = _selected_qp;
  }

  if (_position >= _tensors[qp].size())
  {
    mooseError("VectorRankTwoAux position specified as ",
               _position,
               " but there are only ",
               _tensors[qp].size(),
               " tensors in the list");
  }

  return _tensors[qp][_position](_i, _j);
}
