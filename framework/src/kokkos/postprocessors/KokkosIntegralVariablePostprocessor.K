//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosIntegralVariablePostprocessor.h"

template <typename Base>
InputParameters
KokkosIntegralVariablePostprocessor<Base>::validParams()
{
  InputParameters params = Base::validParams();
  params.addRequiredCoupledVar("variable", "The name of the variable that this object operates on");
  params.addClassDescription("Computes a volume integral of the specified variable");
  params.addParam<bool>(
      "use_absolute_value", false, "Whether to use absolute value of the variable or not");
  return params;
}

template <typename Base>
KokkosIntegralVariablePostprocessor<Base>::KokkosIntegralVariablePostprocessor(
    const InputParameters & parameters)
  : Base(parameters),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _u(Base::kokkosCoupledValue("variable")),
    _grad_u(Base::kokkosCoupledGradient("variable")),
    _use_abs_value(Base::template getParam<bool>("use_absolute_value"))
{
  Base::addMooseVariableDependency(&mooseVariableField());
}

template class KokkosIntegralVariablePostprocessor<KokkosElementIntegralPostprocessor>;
template class KokkosIntegralVariablePostprocessor<KokkosSideIntegralPostprocessor>;
