//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RankTwoAux.h"
#include "RankTwoScalarTools.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("TensorMechanicsApp", RankTwoAux);
registerMooseObject("TensorMechanicsApp", ADRankTwoAux);

template <bool is_ad>
InputParameters
RankTwoAuxTempl<is_ad>::validParams()
{
  InputParameters params = NodalPatchRecovery::validParams();
  params.addClassDescription("Access a component of a RankTwoTensor");
  params.addRequiredParam<MaterialPropertyName>("rank_two_tensor",
                                                "The rank two material tensor name");
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

template <bool is_ad>
RankTwoAuxTempl<is_ad>::RankTwoAuxTempl(const InputParameters & parameters)
  : NodalPatchRecovery(parameters),
    _tensor(getGenericMaterialProperty<RankTwoTensor, is_ad>("rank_two_tensor")),
    _i(getParam<unsigned int>("index_i")),
    _j(getParam<unsigned int>("index_j")),
    _has_selected_qp(isParamValid("selected_qp")),
    _selected_qp(_has_selected_qp ? getParam<unsigned int>("selected_qp") : 0)
{
}

template <bool is_ad>
Real
RankTwoAuxTempl<is_ad>::computeValue()
{
  unsigned int qp = _qp;
  if (_has_selected_qp)
  {
    if (_selected_qp >= _q_point.size())
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      mooseError("RankTwoAux.  selected_qp specified as ",
                 _selected_qp,
                 " but there are only ",
                 _q_point.size(),
                 " quadpoints in the element");
    }
    qp = _selected_qp;
  }

  return RankTwoScalarTools::component(MetaPhysicL::raw_value(_tensor[qp]), _i, _j);
}

template class RankTwoAuxTempl<false>;
template class RankTwoAuxTempl<true>;
