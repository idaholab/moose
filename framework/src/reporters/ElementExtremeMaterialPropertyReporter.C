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

  params.addRequiredParam<MaterialPropertyName>("material_property",
                                                "Material property for which to find the extreme. "
                                                "The value of this property is always reported.");
  MooseEnum type_options("max=0 min=1");
  params.addRequiredParam<MooseEnum>("value_type",
                                     type_options,
                                     "Type of extreme value to report: 'max' "
                                     "reports the maximum value and 'min' reports "
                                     "the minimum value.");

  params.addParam<std::vector<MaterialPropertyName>>(
      "additional_reported_properties",
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
    _mat_prop(getGenericMaterialProperty<Real, is_ad>("material_property")),
    _type(getParam<MooseEnum>("value_type").template getEnum<ExtremeType>()),
    _extreme_value(declareValueByName<Real>("extreme_value")),
    _coordinates(declareValueByName<Point>("coordinates")),
    _qp(0)
{
  const auto & mat_prop_names =
      getParam<std::vector<MaterialPropertyName>>("additional_reported_properties");
  // TODO: Add more options for the type of reported properties. Currently these have to be
  //       either Real (if using the non-AD version of this, or ADReal if using the AD version).
  //       ElementMaterialSampler can get multiple types (int, real, unsigned), maybe do
  //       something like that and also get AD and non-AD verisons.
  for (const auto & mpn : mat_prop_names)
  {
    _additional_reported_properties.push_back(&getGenericMaterialPropertyByName<Real, is_ad>(mpn));
    _additional_reported_property_values.push_back(&declareValueByName<Real>(mpn));
  }
}

template <bool is_ad>
void
ElementExtremeMaterialPropertyReporterTempl<is_ad>::initialize()
{
  switch (_type)
  {
    case ExtremeType::MAX:
      _extreme_value = -std::numeric_limits<Real>::max();
      break;

    case ExtremeType::MIN:
      _extreme_value = std::numeric_limits<Real>::max();
      break;
  }
  _coordinates = Point(0., 0., 0.);
  for (const auto i : index_range(_additional_reported_properties))
    *_additional_reported_property_values[i] = 0.0;
}

template <bool is_ad>
void
ElementExtremeMaterialPropertyReporterTempl<is_ad>::execute()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    computeQpValue();
}

template <bool is_ad>
void
ElementExtremeMaterialPropertyReporterTempl<is_ad>::computeQpValue()
{
  const Real raw_mat_val = MetaPhysicL::raw_value(_mat_prop[_qp]);
  switch (_type)
  {
    case ExtremeType::MAX:
      if (raw_mat_val > _extreme_value)
      {
        _extreme_value = raw_mat_val;
        _coordinates = _q_point[_qp];
        for (const auto i : index_range(_additional_reported_properties))
          *_additional_reported_property_values[i] =
              MetaPhysicL::raw_value((*_additional_reported_properties[i])[_qp]);
      }
      break;

    case ExtremeType::MIN:
      if (raw_mat_val < _extreme_value)
      {
        _extreme_value = raw_mat_val;
        _coordinates = _q_point[_qp];
        for (const auto i : index_range(_additional_reported_properties))
          *_additional_reported_property_values[i] =
              MetaPhysicL::raw_value((*_additional_reported_properties[i])[_qp]);
      }
      break;
  }
}

template <bool is_ad>
void
ElementExtremeMaterialPropertyReporterTempl<is_ad>::finalize()
{
  unsigned int rank = 0;

  switch (_type)
  {
    case ExtremeType::MAX:
      _communicator.maxloc(_extreme_value, rank);
      break;
    case ExtremeType::MIN:
      _communicator.minloc(_extreme_value, rank);
      break;
  }

  const auto prop_size = _additional_reported_property_values.size();
  std::vector<Real> ev_rep_prop_vals(prop_size, 0.);

  _communicator.broadcast(_coordinates, rank);
  if (rank == processor_id())
    for (const auto i : make_range(prop_size))
      ev_rep_prop_vals[i] = *_additional_reported_property_values[i];
  _communicator.broadcast(ev_rep_prop_vals, rank, /*identical_sizes=*/true);
  for (const auto i : make_range(prop_size))
    *_additional_reported_property_values[i] = ev_rep_prop_vals[i];
}

template <bool is_ad>
void
ElementExtremeMaterialPropertyReporterTempl<is_ad>::threadJoin(const UserObject & uo)
{
  const auto & rpt = static_cast<const ElementExtremeMaterialPropertyReporterTempl<is_ad> &>(uo);
  const auto prop_size = _additional_reported_property_values.size();

  switch (_type)
  {
    case ExtremeType::MAX:
      if (rpt._extreme_value > _extreme_value)
      {
        _extreme_value = rpt._extreme_value;
        _coordinates = rpt._coordinates;
        for (const auto i : make_range(prop_size))
          *_additional_reported_property_values[i] = *(rpt._additional_reported_property_values[i]);
      }
      break;

    case ExtremeType::MIN:
      if (rpt._extreme_value < _extreme_value)
      {
        _extreme_value = rpt._extreme_value;
        _coordinates = rpt._coordinates;
        for (const auto i : make_range(prop_size))
          *_additional_reported_property_values[i] = *(rpt._additional_reported_property_values[i]);
      }
      break;
  }
}

template class ElementExtremeMaterialPropertyReporterTempl<false>;
template class ElementExtremeMaterialPropertyReporterTempl<true>;
