//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorTransformFunctorMaterial.h"

#include "libmesh/point_locator_base.h"

registerMooseObject("MooseApp", FunctorTransformFunctorMaterial);
registerMooseObject("MooseApp", ADFunctorTransformFunctorMaterial);

template <bool is_ad>
InputParameters
FunctorTransformFunctorMaterialTempl<is_ad>::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.set<ExecFlagEnum>("execute_on") = {EXEC_ALWAYS};
  params.addClassDescription(
      "FunctorMaterial object for declaring properties that are populated by evaluation of a "
      "Functor (a constant, variable, function or functor material property) objects with an "
      "offset defined by three functors.");
  params.addParam<std::vector<std::string>>("prop_names",
                                            "The names of the properties this material will have");
  params.addParam<std::vector<MooseFunctorName>>("prop_values",
                                                 "The corresponding names of the "
                                                 "functors that are going to provide "
                                                 "the values for the variables");

  // Functors to define the transformation
  params.addParam<MooseFunctorName>(
      "x_functor",
      "Functor providing the X-coordinate. Defaults to the local x coordinate if not specified");
  params.addParam<MooseFunctorName>(
      "y_functor",
      "Functor providing the X-coordinate. Defaults to the local y coordinate if not specified");
  params.addParam<MooseFunctorName>(
      "z_functor",
      "Functor providing the X-coordinate. Defaults to the local z coordinate if not specified");

  return params;
}

template <bool is_ad>
FunctorTransformFunctorMaterialTempl<is_ad>::FunctorTransformFunctorMaterialTempl(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _prop_names(getParam<std::vector<std::string>>("prop_names")),
    _prop_values(getParam<std::vector<MooseFunctorName>>("prop_values")),
    _x_functor(isParamValid("x_functor") ? &getFunctor<Real>("x_functor") : nullptr),
    _y_functor(isParamValid("y_functor") ? &getFunctor<Real>("y_functor") : nullptr),
    _z_functor(isParamValid("z_functor") ? &getFunctor<Real>("z_functor") : nullptr)
{
  unsigned int num_names = _prop_names.size();
  unsigned int num_values = _prop_values.size();

  if (num_names != num_values)
    mooseError("Number of prop_names must match the number of prop_values for a "
               "FunctorTransformFunctorMaterial!");

  // Check that there is no name conflict, a common mistake with this object
  for (const auto i : make_range(num_names))
    for (const auto j : make_range(num_values))
      if (_prop_names[i] == _prop_values[j])
        paramError("prop_names",
                   "prop_names should not be the same as any of the prop_values. They"
                   " can both be functors, and functors may not have the same name.");

  _num_props = num_names;
  _functors.resize(num_names);

  for (const auto i : make_range(_num_props))
  {
    _functors[i] = &getFunctor<GenericReal<is_ad>>(_prop_values[i]);
    if (n_processors() > 1 && _functors[i]->mayRequireGhosting())
      mooseError("Transformed functors not currently supported in parallel as functor '",
                 _prop_values[i],
                 "' may require parallel ghosting for evaluation at offset/transformed location");
  }

  // Retrieve the point locator here once to make sure it's not created during a functor evaluation
  const auto & pl = _mesh.getPointLocator();
  libmesh_ignore(pl);

  const std::set<ExecFlagType> clearance_schedule(_execute_enum.begin(), _execute_enum.end());
  for (const auto i : make_range(_num_props))
  {
    addFunctorProperty<GenericReal<is_ad>>(
        _prop_names[i],
        [this, i](const auto & r, const auto & t) -> GenericReal<is_ad>
        {
          // Gather the components for the new evaluation location
          const auto x = _x_functor ? (*_x_functor)(r, t) : r.getPoint()(0);
          const auto y = _y_functor ? (*_y_functor)(r, t) : r.getPoint()(1);
          const auto z = _z_functor ? (*_z_functor)(r, t) : r.getPoint()(2);
          const auto new_point = Point(x, y, z);
          const auto pl = _mesh.getPointLocator();

          // The transformation impacts different functor argument in different ways
          if constexpr (std::is_same_v<const Moose::ElemArg &, decltype(r)>)
          {
            const auto new_elem = (*pl)(new_point);
            Moose::ElemArg new_r = {new_elem, r.correct_skewness};
            return (*_functors[i])(new_r, t);
          }
          else if constexpr (std::is_same_v<const Moose::ElemPointArg &, decltype(r)>)
          {
            const auto new_elem = (*pl)(new_point);
            Moose::ElemPointArg new_r = {new_elem, new_point, r.correct_skewness};
            return (*_functors[i])(new_r, t);
          }
          else if constexpr (std::is_same_v<const Moose::NodeArg &, decltype(r)>)
          {
            const auto new_node = pl->locate_node(new_point, &this->blockIDs());
            if (this->blockIDs().size() != 1)
              paramError("block", "A single block must be specified for nodal evaluation");
            Moose::NodeArg new_r = {new_node, *this->blockIDs().begin()};
            return (*_functors[i])(new_r, t);
          }
          else
          {
            flagSolutionWarning(
                "Functor argument not current supported: defaulting to an ElemPointArg "
                "with no skewness correction");
            const auto new_elem = (*pl)(new_point);
            Moose::ElemPointArg new_r = {new_elem, new_point, false};
            return (*_functors[i])(new_r, t);
          }
        },
        clearance_schedule);
  }
}

template class FunctorTransformFunctorMaterialTempl<false>;
template class FunctorTransformFunctorMaterialTempl<true>;
