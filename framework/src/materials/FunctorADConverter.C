//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorADConverter.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", FunctorADConverter);
registerMooseObject("MooseApp", VectorFunctorADConverter);

template <typename T>
InputParameters
FunctorADConverterTempl<T>::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("Converts regular functors to AD functors and "
                             "AD functors to regular functors");
  params.addParam<std::vector<MooseFunctorName>>(
      "reg_props_in", "The names of the regular functors to convert to AD functors");
  params.addParam<std::vector<MooseFunctorName>>("ad_props_out",
                                                 "The names of the output AD functors");
  params.addParam<std::vector<MooseFunctorName>>(
      "ad_props_in", "The names of the AD functors to convert to regular functors");
  params.addParam<std::vector<MooseFunctorName>>("reg_props_out",
                                                 "The names of the output regular functors");
  return params;
}

template <typename T>
FunctorADConverterTempl<T>::FunctorADConverterTempl(const InputParameters & parameters)
  : FunctorMaterial(parameters)
{
  auto reg_props_in = getParam<std::vector<MooseFunctorName>>("reg_props_in");
  auto ad_props_out = getParam<std::vector<MooseFunctorName>>("ad_props_out");
  auto ad_props_in = getParam<std::vector<MooseFunctorName>>("ad_props_in");
  auto reg_props_out = getParam<std::vector<MooseFunctorName>>("reg_props_out");

  // Check input sizes
  if (reg_props_in.size() != ad_props_out.size())
    paramError("ad_props_out",
               "The number of output AD functors must match the number of input regular "
               "functors, which is " +
                   std::to_string(reg_props_in.size()));

  if (ad_props_in.size() != reg_props_out.size())
    paramError("reg_props_out",
               "The number of output regular functors must match the number of input AD "
               "functors, which is " +
                   std::to_string(ad_props_in.size()));

  // Check input names for overlaps, before possibly hitting a harder to
  // interpret error at functor definition
  for (const auto i : make_range(reg_props_in.size()))
    for (const auto j : make_range(reg_props_in.size()))
      if (reg_props_in[i] == ad_props_out[j])
        paramError("reg_props_in",
                   "Functor names may not overlap between reg_props_in and ad_props_out");

  for (const auto i : make_range(reg_props_in.size()))
    for (const auto j : make_range(ad_props_in.size()))
      if (reg_props_in[i] == reg_props_out[j])
        paramError("reg_props_in",
                   "Functor names may not overlap between reg_props_in and reg_props_out");

  for (const auto i : make_range(ad_props_in.size()))
    for (const auto j : make_range(ad_props_in.size()))
      if (ad_props_in[i] == reg_props_out[j])
        paramError("ad_props_in",
                   "Functor names may not overlap between ad_props_in and reg_props_out");

  for (const auto i : make_range(ad_props_in.size()))
    for (const auto j : make_range(reg_props_in.size()))
      if (ad_props_in[i] == ad_props_out[j])
        paramError("ad_props_in",
                   "Functor names may not overlap between ad_props_in and ad_props_out");

  // Define the AD functors
  for (const auto i : make_range(reg_props_in.size()))
  {
    const auto & reg_functor = getFunctor<T>(reg_props_in[i]);
    addFunctorProperty<typename Moose::ADType<T>::type>(
        ad_props_out[i],
        [&reg_functor](const auto & r, const auto & t) -> typename Moose::ADType<T>::type
        { return reg_functor(r, t); });
  }

  // Define the regular functors
  for (const auto i : make_range(ad_props_in.size()))
  {
    const auto & ad_functor = getFunctor<typename Moose::ADType<T>::type>(ad_props_in[i]);
    addFunctorProperty<T>(reg_props_out[i],
                          [&ad_functor](const auto & r, const auto & t) -> T
                          { return MetaPhysicL::raw_value(ad_functor(r, t)); });
  }
}

template class FunctorADConverterTempl<Real>;
template class FunctorADConverterTempl<RealVectorValue>;
