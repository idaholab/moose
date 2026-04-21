//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosDirichletBC.h"

registerKokkosDirichletBC("MooseApp", KokkosDirichletBC);
registerKokkosADDirichletBC("MooseApp", KokkosADDirichletBC);

template <bool is_ad>
InputParameters
KokkosDirichletBCTempl<is_ad>::validParams()
{
  InputParameters params = Moose::Kokkos::DirichletBCBaseTempl<is_ad>::validParams();
  params.addRequiredParam<Real>("value", "Value of the BC");
  params.declareControllable("value");
  params.addClassDescription("Imposes the essential boundary condition $u=g$, where $g$ "
                             "is a constant, controllable value.");
  return params;
}

template <bool is_ad>
KokkosDirichletBCTempl<is_ad>::KokkosDirichletBCTempl(const InputParameters & parameters)
  : Moose::Kokkos::DirichletBCBaseTempl<is_ad>(parameters),
    _value(this->template getParam<Real>("value"))
{
}

template class KokkosDirichletBCTempl<false>;
template class KokkosDirichletBCTempl<true>;
