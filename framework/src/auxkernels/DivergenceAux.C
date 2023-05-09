//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DivergenceAux.h"
#include "MooseMesh.h"

registerMooseObject("MooseApp", DivergenceAux);
registerMooseObject("MooseApp", ADDivergenceAux);

template <bool is_ad>
InputParameters
DivergenceAuxTempl<is_ad>::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addClassDescription("Computes the divergence of a vector of functors.");
  // Coupled functors
  params.addRequiredParam<MooseFunctorName>("u", "x-component of the vector");
  params.addParam<MooseFunctorName>("v", "y-component of the vector"); // only required in 2D and 3D
  params.addParam<MooseFunctorName>("w", "z-component of the vector"); // only required in 3D

  return params;
}

template <bool is_ad>
DivergenceAuxTempl<is_ad>::DivergenceAuxTempl(const InputParameters & parameters)
  : AuxKernel(parameters),
    _u(getFunctor<GenericReal<is_ad>>("u")),
    _v(_mesh.dimension() >= 2 ? &getFunctor<GenericReal<is_ad>>("v") : nullptr),
    _w(_mesh.dimension() == 3 ? &getFunctor<GenericReal<is_ad>>("w") : nullptr),
    _use_qp_arg(dynamic_cast<MooseVariableFE<Real> *>(&_var))
{
}

template <bool is_ad>
Real
DivergenceAuxTempl<is_ad>::computeValue()
{
  using MetaPhysicL::raw_value;
  const auto state = determineState();
  Real divergence = 0;
  if (_use_qp_arg)
  {
    const auto qp_arg = std::make_tuple(_current_elem, _qp, _qrule);
    divergence += raw_value(_u.gradient(qp_arg, state)(0));
    if (_v)
      divergence += raw_value(_v->gradient(qp_arg, state)(1));
    if (_w)
      divergence += raw_value(_w->gradient(qp_arg, state)(2));
    return divergence;
  }
  else
  {
    const auto elem_arg = makeElemArg(_current_elem);
    divergence += raw_value(_u.gradient(elem_arg, state)(0));
    if (_v)
      divergence += raw_value(_v->gradient(elem_arg, state)(1));
    if (_w)
      divergence += raw_value(_w->gradient(elem_arg, state)(2));
    return divergence;
  }
}
