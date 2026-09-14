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
    _function(this->template getKokkosFunction<KokkosParsedFunction>("function"))
{
}

template <bool is_ad>
Real
KokkosFunctionDirichletBCTempl<is_ad>::hostValue(const libMesh::Point & p, Real time) const
{
  // The function this holds a reference to cannot be evaluated directly: RPNEvaluator::init()
  // builds the RPN sequence but leaves the storage for the values behind the function's symbols
  // empty, and it is the evaluator's copy constructor that dereferences those symbols into it. So
  // a copy is what is evaluable, and taking one here is what lets a symbol-bearing expression be
  // read on host.
  //
  // The copy is taken per call rather than held as a member because a symbol can be bound to a
  // Postprocessor, whose value moves as the simulation advances, and this is reached once per
  // timestep through Moose::Kokkos::DirichletValueFunction while libMesh's constraint machinery
  // recomputes the values it prescribes.
  return KokkosParsedFunction(*_function).value(time, Moose::Kokkos::Real3(p(0), p(1), p(2)));
}

template class KokkosFunctionDirichletBCTempl<false>;
template class KokkosFunctionDirichletBCTempl<true>;
