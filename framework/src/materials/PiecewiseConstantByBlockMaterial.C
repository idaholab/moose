//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseConstantByBlockMaterial.h"

registerMooseObject("MooseApp", PiecewiseConstantByBlockMaterial);
registerMooseObject("MooseApp", ADPiecewiseConstantByBlockMaterial);

template <bool is_ad>
InputParameters
PiecewiseConstantByBlockMaterialTempl<is_ad>::validParams()
{
  auto params = Material::validParams();
  params.addClassDescription("Computes a property value on a per-subdomain basis");
  // Somehow min gcc doesn't know the type of params here
  params.template addRequiredParam<MaterialPropertyName>("prop_name",
                                                         "The name of the property to declare");
  params.template addRequiredParam<std::map<std::string, Real>>(
      "subdomain_to_prop_value", "Map from subdomain to property value");
  return params;
}

template <bool is_ad>
PiecewiseConstantByBlockMaterialTempl<is_ad>::PiecewiseConstantByBlockMaterialTempl(
    const InputParameters & params)
  : Material(params), _prop(declareGenericProperty<Real, is_ad>("prop_name"))
{
  for (const auto & map_pr : getParam<std::map<std::string, Real>>("subdomain_to_prop_value"))
    _sub_id_to_prop.emplace(std::make_pair(_mesh.getSubdomainID(map_pr.first), map_pr.second));
}

template <bool is_ad>
void
PiecewiseConstantByBlockMaterialTempl<is_ad>::computeQpProperties()
{
  if (!_bnd)
  {
    mooseAssert(_current_elem,
                "We should be on a block which means we should definitely have a current element");
    auto it = _sub_id_to_prop.find(_current_elem->subdomain_id());
    mooseAssert(it != _sub_id_to_prop.end(),
                "Block restriction must match the subdomain names passed in the "
                "subdomain_to_prop_value parameter");
    _prop[_qp] = it->second;

    return;
  }

  if (_current_elem)
  {
    _face_info = _mesh.faceInfo(_current_elem, _current_side);
    if (!_face_info)
    {
      const Elem * const neighbor = _current_elem->neighbor_ptr(_current_side);
      mooseAssert(neighbor, "Should be non-null");
      const auto neighbor_side = neighbor->which_neighbor_am_i(_current_elem);
      _face_info = _mesh.faceInfo(neighbor, neighbor_side);
      mooseAssert(_face_info, "We need to have retrieved something.");
    }
    mooseAssert(_current_elem->build_side_ptr(_current_side)->vertex_average() ==
                    _face_info->faceCentroid(),
                "Making sure we're in the right place");
    auto it = _sub_id_to_prop.find(_current_elem->subdomain_id());
    if (it == _sub_id_to_prop.end())
    {
      // We may be a ghosted material
      const bool current_elem_is_fi_elem = _current_elem == &_face_info->elem();
      const Elem * const other_elem_to_try =
          current_elem_is_fi_elem ? _face_info->neighborPtr() : &_face_info->elem();
      mooseAssert(other_elem_to_try, "This should be non-null");
      it = _sub_id_to_prop.find(other_elem_to_try->subdomain_id());
      mooseAssert(it != _sub_id_to_prop.end(),
                  "Block restriction must match the subdomain names passed in the "
                  "subdomain_to_prop_value parameter");
    }
    _prop[_qp] = it->second;
    return;
  }

  mooseAssert(
      _face_info,
      "We must have set a face info object in order for the PiecewiseConstantByBlockMaterial "
      "class to work on faces");

  // We must be off the domain
  auto it = _sub_id_to_prop.find(_face_info->elem().subdomain_id());
  mooseAssert(it != _sub_id_to_prop.end(),
              "Block restriction must match the subdomain names passed in the "
              "subdomain_to_prop_value parameter");
  _prop[_qp] = it->second;
}

template class PiecewiseConstantByBlockMaterialTempl<false>;
template class PiecewiseConstantByBlockMaterialTempl<true>;
