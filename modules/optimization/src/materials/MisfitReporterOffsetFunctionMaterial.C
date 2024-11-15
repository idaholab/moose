//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MisfitReporterOffsetFunctionMaterial.h"

#include "libmesh/libmesh_common.h"

registerMooseObject("OptimizationApp", MisfitReporterOffsetFunctionMaterial);
registerMooseObject("OptimizationApp", ADMisfitReporterOffsetFunctionMaterial);

template <bool is_ad>
InputParameters
MisfitReporterOffsetFunctionMaterialTempl<is_ad>::validParams()
{
  InputParameters params = ReporterOffsetFunctionMaterial::validParams();
  params.addClassDescription(
      "Computes the misfit and misfit gradient materials for inverse optimizations problems.");

  params.addRequiredCoupledVar("forward_variable",
                               "Variable that is being used for the forward simulation.");
  params.addRequiredParam<ReporterName>("measurement_value_name",
                                        "Reporter with measurement data.");
  return params;
}

template <bool is_ad>
MisfitReporterOffsetFunctionMaterialTempl<is_ad>::MisfitReporterOffsetFunctionMaterialTempl(
    const InputParameters & parameters)
  : ReporterOffsetFunctionMaterialTempl<is_ad>(parameters),
    _sim_var(this->template coupledGenericValue<is_ad>("forward_variable")),
    _mat_prop_gradient(
        this->template declareGenericProperty<Real, is_ad>(_prop_name + "_gradient")),
    _measurement_values(this->template getReporterValue<std::vector<Real>>(
        "measurement_value_name", REPORTER_MODE_REPLICATED))
{
}

template <bool is_ad>
void
MisfitReporterOffsetFunctionMaterialTempl<is_ad>::computeQpProperties()
{
  _material[_qp] = 0.0;
  _mat_prop_gradient[_qp] = 0.0;
  auto num_pts = _read_in_points ? _points.size() : _coordx.size();
  if (!_read_in_points)
    mooseAssert((_coordx.size() == _coordy.size()) && (_coordx.size() == _coordz.size()),
                "Size of the coordinate offsets don't match.");

  mooseAssert(num_pts == _measurement_values.size(),
              "Number of offsets doesn't match the number of measurements.");

  for (const auto idx : make_range(num_pts))
  {
    const Point offset =
        _read_in_points ? _points[idx] : Point(_coordx[idx], _coordy[idx], _coordz[idx]);

    const Real measurement_value = _measurement_values[idx];
    const auto simulation_value = _sim_var[_qp];

    // Compute weighting function
    const Real weighting = computeOffsetFunction(offset);

    // Computed weighted misfit and gradient materials
    _material[_qp] +=
        Utility::pow<2>(weighting) * Utility::pow<2>(measurement_value - simulation_value);
    _mat_prop_gradient[_qp] -=
        2.0 * Utility::pow<2>(weighting) * (measurement_value - simulation_value);
  }
}

template class MisfitReporterOffsetFunctionMaterialTempl<true>;
template class MisfitReporterOffsetFunctionMaterialTempl<false>;
