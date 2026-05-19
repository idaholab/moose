//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosDirichletBCBase.h"

namespace Moose::Kokkos
{

template <bool is_ad>
InputParameters
DirichletBCBaseTempl<is_ad>::validParams()
{
  InputParameters params = Base::validParams();
  params.addParam<bool>(
      "preset", true, "Whether or not to preset the BC (apply the value before the solve begins).");
  return params;
}

template <bool is_ad>
DirichletBCBaseTempl<is_ad>::DirichletBCBaseTempl(const InputParameters & parameters)
  : Base(parameters), _preset(this->template getParam<bool>("preset"))
{
}

template <bool is_ad>
void
DirichletBCBaseTempl<is_ad>::presetSolution(TagID tag)
{
  _solution_tag = tag;

  Policy policy(0, numKokkosBoundaryNodes());

  auto dispatcher = DispatcherRegistry::build<PresetLoop>(this, this->type());

  dispatcher->parallelFor(policy);
}

template class DirichletBCBaseTempl<false>;
template class DirichletBCBaseTempl<true>;

} // namespace Moose::Kokkos
