//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementExtremeMaterialPropertyReporter.h"

#include "metaphysicl/raw_type.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_algebra.h"

#include <algorithm>
#include <limits>

registerMooseObject("MooseApp", ElementExtremeMaterialPropertyReporter);
registerMooseObject("MooseApp", ADElementExtremeMaterialPropertyReporter);

template <bool is_ad>
InputParameters
ElementExtremeMaterialPropertyReporterTempl<is_ad>::validParams()
{
  InputParameters params = ElementReporter::validParams();

  params.addRequiredParam<MaterialPropertyName>("mat_prop",
                                                "Material property for which to find the extreme");
  MooseEnum type_options("max=0 min=1");
  params.addRequiredParam<MooseEnum>("value_type",
                                     type_options,
                                     "Type of extreme value to return: 'max' "
                                     "returns the maximum value and 'min' returns "
                                     "the minimum value.");

  params.addParam<std::vector<MaterialPropertyName>>(
      "reported_properties",
      {},
      "Additional material properties reported at the location of the extreme value");
  params.addClassDescription(
      "Determines the location of the minimum or maximum value of a material property over a "
      "volume, and provides its coordinates and optionally other requested data at that location.");

  return params;
}

template <bool is_ad>
ElementExtremeMaterialPropertyReporterTempl<is_ad>::ElementExtremeMaterialPropertyReporterTempl(
    const InputParameters & parameters)
  : ElementReporter(parameters),

    _mat_prop(getGenericMaterialProperty<Real, is_ad>("mat_prop")),
    _type((ExtremeType)(int)parameters.get<MooseEnum>("value_type")),
    _extreme_value(declareValueByName<Real>("extreme_value")),
    _coordinates(declareValueByName<Point>("coordinates")),
    _qp(0)
{
  const auto mat_prop_names = getParam<std::vector<MaterialPropertyName>>("reported_properties");
  // TODO: Add more options for the type of reported properties. Currently these have to be
  //       either Real (if using the non-AD version of this, or ADReal if using the AD version).
  //       ElementMaterialSampler can get multiple types (int, real, unsigned), maybe do
  //       something like that and also get AD and non-AD verisons.
  for (const auto mpn : mat_prop_names)
  {
    _reported_properties.push_back(&getGenericMaterialPropertyByName<Real, is_ad>(mpn));
    _reported_property_values.push_back(&declareValueByName<Real>(mpn));
  }
}

template <bool is_ad>
void
ElementExtremeMaterialPropertyReporterTempl<is_ad>::initialize()
{
  switch (_type)
  {
    case MAX:
      _extreme_value = -std::numeric_limits<Real>::max();
      break;

    case MIN:
      _extreme_value = std::numeric_limits<Real>::max();
      break;
  }
}

template <bool is_ad>
void
ElementExtremeMaterialPropertyReporterTempl<is_ad>::execute()
{
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    computeQpValue();
}

template <bool is_ad>
void
ElementExtremeMaterialPropertyReporterTempl<is_ad>::computeQpValue()
{
  Real raw_mat_val = MetaPhysicL::raw_value(_mat_prop[_qp]);
  switch (_type)
  {
    case MAX:
      if (raw_mat_val > _extreme_value)
      {
        _extreme_value = raw_mat_val;
        _coordinates = _q_point[_qp];
        for (unsigned int i = 0; i < _reported_properties.size(); ++i)
          *_reported_property_values[i] = MetaPhysicL::raw_value((*_reported_properties[i])[_qp]);
      }
      break;

    case MIN:
      if (raw_mat_val < _extreme_value)
      {
        _extreme_value = raw_mat_val;
        _coordinates = _q_point[_qp];
        for (unsigned int i = 0; i < _reported_properties.size(); ++i)
          *_reported_property_values[i] = MetaPhysicL::raw_value((*_reported_properties[i])[_qp]);
      }
      break;
  }
}

template <bool is_ad>
void
ElementExtremeMaterialPropertyReporterTempl<is_ad>::finalize()
{
  unsigned int rank;
  Real ev_value = _extreme_value;

  switch (_type)
  {
    case MAX:
      _communicator.maxloc(ev_value, rank);
      break;
    case MIN:
      _communicator.minloc(ev_value, rank);
      break;
  }
  _extreme_value = ev_value;

  Point ev_coord(0., 0., 0.);
  const auto prop_size = _reported_property_values.size();
  std::vector<Real> ev_rep_prop_vals(prop_size, 0.);
  if (rank == processor_id())
  {
    ev_coord = _coordinates;
    for (const auto i : make_range(prop_size))
      ev_rep_prop_vals[i] = *_reported_property_values[i];
  }
  _communicator.sum(ev_coord);
  _coordinates = ev_coord;
  for (const auto i : make_range(prop_size))
  {
    _communicator.sum(ev_rep_prop_vals[i]);
    *_reported_property_values[i] = ev_rep_prop_vals[i];
  }
}

template <bool is_ad>
void
ElementExtremeMaterialPropertyReporterTempl<is_ad>::threadJoin(const UserObject & uo)
{
  const auto & rpt = static_cast<const ElementExtremeMaterialPropertyReporterTempl<is_ad> &>(uo);
  const auto prop_size = _reported_property_values.size();

  switch (_type)
  {
    case MAX:
      if (rpt._extreme_value > _extreme_value)
      {
        _extreme_value = rpt._extreme_value;
        _coordinates = rpt._coordinates;
        for (const auto i : make_range(prop_size))
          *_reported_property_values[i] = *(rpt._reported_property_values[i]);
      }
      break;

    case MIN:
      if (rpt._extreme_value < _extreme_value)
      {
        _extreme_value = rpt._extreme_value;
        _coordinates = rpt._coordinates;
        for (const auto i : make_range(prop_size))
          *_reported_property_values[i] = *(rpt._reported_property_values[i]);
      }
      break;
  }
}

template class ElementExtremeMaterialPropertyReporterTempl<false>;
template class ElementExtremeMaterialPropertyReporterTempl<true>;
