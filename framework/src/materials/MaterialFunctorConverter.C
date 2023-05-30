//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialFunctorConverter.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", MaterialFunctorConverter);
registerMooseObject("MooseApp", VectorMaterialFunctorConverter);

template <typename T>
InputParameters
MaterialFunctorConverterTempl<T>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Converts functor to non-AD and AD regular material properties");
  params.addParam<std::vector<MooseFunctorName>>(
      "functors_in", "The names of the functors to convert to regular material properties");
  params.addParam<std::vector<MaterialPropertyName>>("ad_props_out",
                                                     "The names of the output AD properties");
  params.addParam<std::vector<MaterialPropertyName>>("reg_props_out",
                                                     "The names of the output regular properties");
  return params;
}

template <typename T>
MaterialFunctorConverterTempl<T>::MaterialFunctorConverterTempl(const InputParameters & parameters)
  : Material(parameters),
    _num_functors_to_convert(getParam<std::vector<MooseFunctorName>>("functors_in").size())
{
  const auto & functors_in = getParam<std::vector<MooseFunctorName>>("functors_in");
  const auto & reg_props_out = getParam<std::vector<MaterialPropertyName>>("reg_props_out");
  const auto & ad_props_out = getParam<std::vector<MaterialPropertyName>>("ad_props_out");

  if (isParamValid("reg_props_out") && isParamValid("ad_props_out"))
    paramError("reg_props_out",
               "We dont support converting functors to both regular and AD material properties in "
               "a single instance of '",
               type(),
               "'. Please create two instances, one for regular and one for AD.");

  if (functors_in.size() != reg_props_out.size() && functors_in.size() != ad_props_out.size())
    paramError(
        "functors_in",
        "The number of output properties must match the number of input functors, which is " +
            std::to_string(functors_in.size()));

  _functors_in.resize(_num_functors_to_convert);
  _ad_props_out.resize(ad_props_out.size());
  _reg_props_out.resize(reg_props_out.size());

  for (const auto i : make_range(_num_functors_to_convert))
    _functors_in[i] = &getFunctor<Moose::GenericType<T, true>>(functors_in[i]);

  for (const auto i : index_range(ad_props_out))
    _ad_props_out[i] = &declareADProperty<T>(ad_props_out[i]);

  for (const auto i : index_range(reg_props_out))
    _reg_props_out[i] = &declareProperty<T>(reg_props_out[i]);
}

template <typename T>
void
MaterialFunctorConverterTempl<T>::initQpStatefulProperties()
{
  computeQpProperties();
}

template <typename T>
void
MaterialFunctorConverterTempl<T>::computeQpProperties()
{
  // Using Qp 0 can leverage the functor caching
  // TODO: Find a way to effectively use subdomain-constant-ness
  unsigned int qp_used = (_constant_option == ConstantTypeEnum::NONE) ? _qp : 0;

  const auto state = Moose::currentState();
  if (_bnd)
  {
    const Moose::ElemSideQpArg side_arg = {_current_elem, _current_side, qp_used, _qrule};
    for (const auto i : index_range(_ad_props_out))
      (*_ad_props_out[i])[_qp] = (*_functors_in[i])(side_arg, state);

    for (const auto i : index_range(_reg_props_out))
      (*_reg_props_out[i])[_qp] = MetaPhysicL::raw_value((*_functors_in[i])(side_arg, state));
  }
  else
  {
    const Elem * elem = _neighbor ? _current_elem->neighbor_ptr(_current_side) : _current_elem;
    mooseAssert(elem, "We should have an element");
    const Moose::ElemQpArg elem_arg = {elem, qp_used, _qrule};
    for (const auto i : index_range(_ad_props_out))
      (*_ad_props_out[i])[_qp] = (*_functors_in[i])(elem_arg, state);

    for (const auto i : index_range(_reg_props_out))
      (*_reg_props_out[i])[_qp] = MetaPhysicL::raw_value((*_functors_in[i])(elem_arg, state));
  }
}

template class MaterialFunctorConverterTempl<Real>;
template class MaterialFunctorConverterTempl<RealVectorValue>;
