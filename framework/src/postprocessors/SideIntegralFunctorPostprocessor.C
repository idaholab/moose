//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideIntegralFunctorPostprocessor.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", SideIntegralFunctorPostprocessor);
registerMooseObject("MooseApp", ADSideIntegralFunctorPostprocessor);

template <bool is_ad>
InputParameters
SideIntegralFunctorPostprocessorTempl<is_ad>::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addRequiredParam<MooseFunctorName>(
      "functor", "The name of the functor that this postprocessor integrates");
  params.addParam<MooseFunctorName>("prefactor", 1, "Factor multiplying the integrand");
  params.addParam<bool>("restrict_to_functors_domain",
                        false,
                        "If the functor (and the prefactor) is only defined only along part of "
                        "the sideset, allows to skip the parts where it is not defined. Please "
                        "keep in mind that if the sideset is defined with the wrong normal, this "
                        "may allow to skip the entire integral.");
  MooseEnum functor_args("qp face", "face");
  params.addParam<MooseEnum>(
      "functor_argument", functor_args, "Location to evaluate the functors at");
  params.addClassDescription(
      "Computes a surface integral of the specified functor, using the "
      "single-sided face argument, which usually means that the functor will be"
      " evaluated from a single side of the surface, not interpolated between "
      "both sides.");
  return params;
}

template <bool is_ad>
SideIntegralFunctorPostprocessorTempl<is_ad>::SideIntegralFunctorPostprocessorTempl(
    const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _functor(getFunctor<GenericReal<is_ad>>("functor")),
    _prefactor(getFunctor<GenericReal<is_ad>>("prefactor")),
    _partial_integral(getParam<bool>("restrict_to_functors_domain"))
{
  _qp_integration = (getParam<MooseEnum>("functor_argument") == "qp");
}

template <bool is_ad>
Real
SideIntegralFunctorPostprocessorTempl<is_ad>::computeFaceInfoIntegral(const FaceInfo * fi)
{
  mooseAssert(fi, "We should have a FaceInfo");
  const auto face = makeCDFace(*fi);
  return computeLocalContribution(face);
}

template <bool is_ad>
template <typename T>
Real
SideIntegralFunctorPostprocessorTempl<is_ad>::computeLocalContribution(const T & functor_arg) const
{
  // It's possible the functor is not defined on that side of the boundary
  // We wont allow that case, unless explicitly requested by the user,
  // as this means the sideset is reversed, but we will allow the case where
  // both sides are defined
  const bool has_elem = checkFunctorDefinedOnSideBlock();

  if (has_elem)
    return MetaPhysicL::raw_value(_prefactor(functor_arg, determineState()) *
                                  _functor(functor_arg, determineState()));
  else
  {
    if (!_partial_integral)
      errorFunctorNotDefinedOnSideBlock();
    return 0;
  }
}

template <bool is_ad>
Real
SideIntegralFunctorPostprocessorTempl<is_ad>::computeQpIntegral()
{
  Moose::ElemSideQpArg elem_side_qp = {_current_elem, _current_side, _qp, _qrule};
  return computeLocalContribution(elem_side_qp);
}

template <bool is_ad>
bool
SideIntegralFunctorPostprocessorTempl<is_ad>::checkFunctorDefinedOnSideBlock() const
{
  if (_functor.hasBlocks(_current_elem->subdomain_id()) &&
      _prefactor.hasBlocks(_current_elem->subdomain_id()))
    return true;
  else
  {
#ifndef NDEBUG
    const auto neighbor_ptr = _current_elem->neighbor_ptr(_current_side);
    if (neighbor_ptr)
    {
      mooseAssert(_functor.hasBlocks(_current_elem->subdomain_id()) ||
                      _functor.hasBlocks(neighbor_ptr->subdomain_id()),
                  "Functor should be defined on at least one side of the boundary");
      mooseAssert(_prefactor.hasBlocks(_current_elem->subdomain_id()) ||
                      _prefactor.hasBlocks(neighbor_ptr->subdomain_id()),
                  "Prefactor should be defined on at least one side of the boundary");
    }
#endif
    return false;
  }
}

template <bool is_ad>
void
SideIntegralFunctorPostprocessorTempl<is_ad>::errorFunctorNotDefinedOnSideBlock() const
{
  paramError("boundary",
             "Functor " + _functor.functorName() + " (or prefactor " + _prefactor.functorName() +
                 ") is not defined on block " + std::to_string(_current_elem->subdomain_id()) +
                 ". Is the functor defined along the whole sideset? "
                 "Are the sidesets in 'boundary' all oriented correctly?");
}

template <bool is_ad>
bool
SideIntegralFunctorPostprocessorTempl<is_ad>::hasFaceSide(const FaceInfo & fi,
                                                          const bool fi_elem_side) const
{
  return _current_elem == (fi_elem_side ? &fi.elem() : fi.neighborPtr());
}

template class SideIntegralFunctorPostprocessorTempl<false>;
template class SideIntegralFunctorPostprocessorTempl<true>;
