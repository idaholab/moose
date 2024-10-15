#include "GaussianMisfit.h"
#include "libmesh/int_range.h"
#include "libmesh/libmesh_common.h"

registerMooseObject("OptimizationApp", GaussianMisfit);

InputParameters
GaussianMisfit::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("");

  params.addRequiredCoupledVar("sim_variable",
                               "Variable that is being for the simulation variable.");
  params.addRequiredParam<ReporterName>(
      "value_name", "reporter value name.  This uses the reporter syntax <reporter>/<name>.");
  params.addParam<ReporterName>(
      "x_coord_name",
      "reporter x-coordinate name.  This uses the reporter syntax <reporter>/<name>.");
  params.addParam<ReporterName>(
      "y_coord_name",
      "reporter y-coordinate name.  This uses the reporter syntax <reporter>/<name>.");
  params.addParam<ReporterName>(
      "z_coord_name",
      "reporter z-coordinate name.  This uses the reporter syntax <reporter>/<name>.");
  params.addParam<ReporterName>("point_name",
                                "reporter point name.  This uses the reporter syntax "
                                "<reporter>/<name>.");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<Real>("beam_width", "Width of the beam");
  return params;
}

GaussianMisfit::GaussianMisfit(const InputParameters & parameters)
  : Material(parameters),
    ReporterInterface(this),
    _sim_var(coupledValue("sim_variable")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _misfit(declarePropertyByName<Real>(_base_name + "_misfit")),
    _misfit_gradient(declarePropertyByName<Real>(_base_name + "_misfit_gradient")),
    _read_in_points(isParamValid("point_name")),
    _measurement_values(
        getReporterValue<std::vector<Real>>("value_name", REPORTER_MODE_REPLICATED)),
    _coordx(isParamValid("x_coord_name")
                ? getReporterValue<std::vector<Real>>("x_coord_name", REPORTER_MODE_REPLICATED)
                : _zeros_vec),
    _coordy(isParamValid("y_coord_name")
                ? getReporterValue<std::vector<Real>>("y_coord_name", REPORTER_MODE_REPLICATED)
                : _zeros_vec),
    _coordz(isParamValid("z_coord_name")
                ? getReporterValue<std::vector<Real>>("z_coord_name", REPORTER_MODE_REPLICATED)
                : _zeros_vec),
    _measurement_points(_read_in_points ? getReporterValue<std::vector<Point>>(
                                              "point_name", REPORTER_MODE_REPLICATED)
                                        : _zeros_pts),
    _beam_width(getParam<Real>("beam_width"))
{
}

void
GaussianMisfit::computeQpProperties()
{
  _misfit[_qp] = 0.0;
  _misfit_gradient[_qp] = 0.0;

  for (const auto idx : index_range(_measurement_values))
  {
    // Get measurement point
    Point p_m(0);
    if (_read_in_points)
      p_m = _measurement_points[idx];
    else
      p_m = Point(_coordx[idx], _coordy[idx], _coordz[idx]);

    Point p_sim = _q_point[_qp];
    Real value_m = _measurement_values[idx];
    Real value_sim = _sim_var[_qp];

    // Compute Gaussian value
    Real guass_value = computeGuassian(p_m, p_sim);

    // Calculate misfit and misfit gradient
    Real diff = value_m - value_sim;
    _misfit[_qp] += diff * guass_value;
    _misfit_gradient[_qp] -= guass_value;
  }
}

Real
GaussianMisfit ::computeGuassian(const Point & pt_measured, const Point & pt_sim)
{
  Real distance_squared = (pt_measured - pt_sim).norm_sq();
  return std::exp(-2.0 * distance_squared / (_beam_width * _beam_width));
}
