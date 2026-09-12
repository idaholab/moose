//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosFunctionDirichletBC.h"

registerKokkosDirichletBC("MooseApp", KokkosFunctionDirichletBC);
registerKokkosADDirichletBC("MooseApp", KokkosADFunctionDirichletBC);

template <bool is_ad>
InputParameters
KokkosFunctionDirichletBCTempl<is_ad>::validParams()
{
  InputParameters params = Moose::Kokkos::DirichletBCBaseTempl<is_ad>::validParams();
  params.addRequiredParam<FunctionName>("function", "The forcing function.");
  params.addClassDescription(
      "Imposes the essential boundary condition $u=g(t,\\vec{x})$, where $g$ "
      "is a (possibly) time and space-dependent MOOSE Function.");
  return params;
}

template <bool is_ad>
KokkosFunctionDirichletBCTempl<is_ad>::KokkosFunctionDirichletBCTempl(
    const InputParameters & parameters)
  : Moose::Kokkos::DirichletBCBaseTempl<is_ad>(parameters),
    _function(this->getKokkosFunction("function"))
{
}

template class KokkosFunctionDirichletBCTempl<false>;
template class KokkosFunctionDirichletBCTempl<true>;
