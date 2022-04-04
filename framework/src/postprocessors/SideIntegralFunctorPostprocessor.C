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
registerMooseObject("MooseApp", SideIntegralADFunctorPostprocessor);

template <bool is_ad>
InputParameters
SideIntegralFunctorPostprocessorTempl<is_ad>::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addRequiredParam<MooseFunctorName>(
      "functor", "The name of the functor that this postprocessor integrates");
  params.addParam<MooseFunctorName>("factor", 1, "Factor multiplying the integrand");
  params.addClassDescription("Computes a surface integral of the specified functor, "
                             "using the single-sided face argument");
  return params;
}

template <bool is_ad>
SideIntegralFunctorPostprocessorTempl<is_ad>::SideIntegralFunctorPostprocessorTempl(
    const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _functor(getFunctor<GenericReal<is_ad>>("functor")),
    _prefactor(getFunctor<GenericReal<is_ad>>("factor"))
{
}

template <bool is_ad>
Real
SideIntegralFunctorPostprocessorTempl<is_ad>::computeQpIntegral()
{
  const FaceInfo * const fi = _mesh.faceInfo(_current_elem, _current_side);
  mooseAssert(fi, "We should have an fi");

  auto aa = _functor.blockIDs();

  // Functor may not be defined on that side of the boundary
  bool use_elem;
  if (_functor.hasBlocks(_current_elem->subdomain_id()))
    use_elem = true;
  else
  {
    mooseAssert(_functor.hasBlocks(fi->neighborPtr()->subdomain_id()),
                "Functor should be defined on one side of the boundary");
    use_elem = false;
  }

  Moose::SingleSidedFaceArg ssf;
  if (use_elem)
    ssf = {*fi,
           Moose::FV::LimiterType::CentralDifference,
           true,
           false,
           false,
           _current_elem->subdomain_id()};
  else
    ssf = {*fi,
           Moose::FV::LimiterType::CentralDifference,
           true,
           false,
           false,
           fi->neighborPtr()->subdomain_id()};

  return MetaPhysicL::raw_value(_prefactor(ssf) * _functor(ssf));
}

template class SideIntegralFunctorPostprocessorTempl<false>;
template class SideIntegralFunctorPostprocessorTempl<true>;
