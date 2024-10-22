#include "MisfitReporterOffsetFunctionMaterial.h"
#include "MooseError.h"
#include "libmesh/int_range.h"
#include "libmesh/libmesh_common.h"

registerMooseObject("OptimizationApp", MisfitReporterOffsetFunctionMaterial);

InputParameters
MisfitReporterOffsetFunctionMaterial::validParams()
{
  InputParameters params = ReporterOffsetFunctionMaterial::validParams();
  params.addClassDescription(
      "Computes the misfit and misfit gradient materials for inverse optimizations problems.");

  params.addRequiredCoupledVar("sim_variable",
                               "Variable that is being for the simulation variable.");
  params.addRequiredParam<ReporterName>(
      "value_name", "reporter value name.  This uses the reporter syntax <reporter>/<name>.");
  return params;
}

MisfitReporterOffsetFunctionMaterial::MisfitReporterOffsetFunctionMaterial(
    const InputParameters & parameters)
  : ReporterOffsetFunctionMaterial(parameters),
    _sim_var(coupledValue("sim_variable")),
    _mat_prop_gradient(declareProperty<Real>(_prop_name + "_gradient")),
    _measurement_values(getReporterValue<std::vector<Real>>("value_name", REPORTER_MODE_REPLICATED))
{
}

void
MisfitReporterOffsetFunctionMaterial::computeQpProperties()
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

    Real measurement_value = _measurement_values[idx];
    Real simulation_value = _sim_var[_qp];

    // Compute weighting function
    Real weighting = computeOffsetFunction(offset);

    // Computed weighted misfit and gradient materials
    _material[_qp] += Utility::pow<2>(measurement_value * weighting - simulation_value * weighting);
    _mat_prop_gradient[_qp] -=
        2.0 * weighting * (measurement_value * weighting - simulation_value * weighting);
  }
}
