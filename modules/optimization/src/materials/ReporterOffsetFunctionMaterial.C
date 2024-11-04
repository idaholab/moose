//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterOffsetFunctionMaterial.h"

registerMooseObject("OptimizationApp", ReporterOffsetFunctionMaterial);
registerMooseObject("OptimizationApp", ADReporterOffsetFunctionMaterial);

template <bool is_ad>
InputParameters
ReporterOffsetFunctionMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute the sum of a function offset by a set of points.");
  params.addRequiredParam<FunctionName>("function", "The weighting function.");

  params.addParam<ReporterName>("x_coord_name", "Reporter with x-coordinate data.");
  params.addParam<ReporterName>("y_coord_name", "Reporter with y-coordinate data.");
  params.addParam<ReporterName>("z_coord_name", "Reporter with z-coordinate data.");
  params.addParam<ReporterName>("point_name",
                                "Reporter with point data. "
                                "<reporter>/<name>.");
  params.addRequiredParam<std::string>("property_name", "Material property base name");
  params.addParamNamesToGroup("point_name x_coord_name y_coord_name z_coord_name",
                              "Offset locations for function evaluations");
  return params;
}

template <bool is_ad>
ReporterOffsetFunctionMaterialTempl<is_ad>::ReporterOffsetFunctionMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),
    ReporterInterface(this),
    _prop_name(getParam<std::string>("property_name")),
    _material(declareGenericProperty<Real, is_ad>(_prop_name)),
    _read_in_points(isParamValid("point_name")),
    _coordx(isParamValid("x_coord_name")
                ? getReporterValue<std::vector<Real>>("x_coord_name", REPORTER_MODE_REPLICATED)
                : _zeros_vec),
    _coordy(isParamValid("y_coord_name")
                ? getReporterValue<std::vector<Real>>("y_coord_name", REPORTER_MODE_REPLICATED)
                : _zeros_vec),
    _coordz(isParamValid("z_coord_name")
                ? getReporterValue<std::vector<Real>>("z_coord_name", REPORTER_MODE_REPLICATED)
                : _zeros_vec),
    _points(_read_in_points
                ? getReporterValue<std::vector<Point>>("point_name", REPORTER_MODE_REPLICATED)
                : _zeros_pts),
    _func(getFunction("function"))
{
  if (isParamValid("point_name") == (isParamValid("x_coord_name") && isParamValid("y_coord_name") &&
                                     isParamValid("z_coord_name")))
    paramError("Either supply x,y, and z reporters or a point reporter.");
}

template <bool is_ad>
void
ReporterOffsetFunctionMaterialTempl<is_ad>::computeQpProperties()
{
  _material[_qp] = 0;
  auto num_pts = _read_in_points ? _points.size() : _coordx.size();
  if (!_read_in_points)
    mooseAssert((_coordx.size() == _coordy.size()) && (_coordx.size() == _coordz.size()),
                "Size of the coordinate offsets don't match.");

  for (const auto idx : make_range(num_pts))
  {

    Point offset = _read_in_points ? _points[idx] : Point(_coordx[idx], _coordy[idx], _coordz[idx]);

    _material[_qp] += computeOffsetFunction(offset);
  }
}

template <bool is_ad>
Real
ReporterOffsetFunctionMaterialTempl<is_ad>::computeOffsetFunction(const Point & point_offset)
{
  return _func.value(_t, _q_point[_qp] - point_offset);
}

template class ReporterOffsetFunctionMaterialTempl<true>;
template class ReporterOffsetFunctionMaterialTempl<false>;
