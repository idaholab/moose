//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVThermalResistanceBC.h"
#include "HeatConductionNames.h"

registerMooseObject("HeatConductionApp", FVThermalResistanceBC);

InputParameters
FVThermalResistanceBC::validParams()
{
  auto params = FVFluxBC::validParams();
  params.addCoupledVar("temperature", "temperature variable");
  params.addRequiredParam<Real>(HeatConduction::T_ambient, "constant ambient temperature");
  params.addRequiredParam<MaterialPropertyName>("htc", "heat transfer coefficient");

  params.addRequiredRangeCheckedParam<Real>(HeatConduction::emissivity,
                                            HeatConduction::emissivity + " >= 0.0 & " +
                                                HeatConduction::emissivity + " <= 1.0",
                                            "emissivity of the surface");

  params.addRequiredParam<std::vector<Real>>(
      "thermal_conductivities",
      "vector of thermal conductivity values used for the conduction layers");
  params.addRequiredParam<std::vector<Real>>("conduction_thicknesses",
                                             "vector of conduction layer thicknesses");

  MooseEnum geometry("cartesian cylindrical", "cartesian");
  params.addParam<MooseEnum>("geometry", geometry, "type of geometry");
  params.addRangeCheckedParam<Real>("inner_radius",
                                    "inner_radius > 0.0",
                                    "coordinate corresponding to the first resistance layer");

  params.addRangeCheckedParam<Real>(
      "step_size", 0.1, "step_size > 0.0", "underrelaxation step size");

  params.addRangeCheckedParam<unsigned int>(
      "max_iterations", 100, "max_iterations >= 0", "maximum iterations");

  params.addRangeCheckedParam<Real>(
      "tolerance", 1E-3, "tolerance > 0.0", "tolerance to converge iterations");
  params.addClassDescription("Thermal resistance Heat flux boundary condition for the "
                             "fluid and solid energy equations");
  return params;
}

FVThermalResistanceBC::FVThermalResistanceBC(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _geometry(getParam<MooseEnum>("geometry").getEnum<Moose::CoordinateSystemType>()),
    _inner_radius(
        _geometry == Moose::CoordinateSystemType::COORD_RZ ? getParam<Real>("inner_radius") : 1.0),
    _T(isParamValid("temperature") ? adCoupledValue("temperature") : _u),
    _T_ambient(getParam<Real>(HeatConduction::T_ambient)),
    _k(getParam<std::vector<Real>>("thermal_conductivities")),
    _dx(getParam<std::vector<Real>>("conduction_thicknesses")),
    _h(getADMaterialPropertyByName<Real>(getParam<MaterialPropertyName>("htc"))),
    _emissivity(getParam<Real>(HeatConduction::emissivity)),
    _max_iterations(getParam<unsigned int>("max_iterations")),
    _tolerance(getParam<Real>("tolerance")),
    _alpha(getParam<Real>("step_size")),
    _T_surface(0.0),
    _outer_radius(_inner_radius),
    _conduction_resistance(0.0),
    _parallel_resistance(0.0)
{
  if (_k.size() != _dx.size())
    paramError("conduction_thicknesses",
               "Number of specified thermal conductivities must match "
               "the number of conduction layers!");

  if (_geometry == Moose::CoordinateSystemType::COORD_RZ)
    for (const auto & d : _dx)
      _outer_radius += d;

  // because the thermal conductivities are constant, we only need to compute
  // the conduction resistance one time
  computeConductionResistance();
}

void
FVThermalResistanceBC::computeConductionResistance()
{
  Real r = _inner_radius;

  for (const auto i : index_range(_k))
  {
    switch (_geometry)
    {
      case Moose::CoordinateSystemType::COORD_XYZ:
        _conduction_resistance += _dx[i] / _k[i];
        break;
      case Moose::CoordinateSystemType::COORD_RZ:
      {
        _conduction_resistance += std::log((_dx[i] + r) / r) / _k[i];
        r += _dx[i];
        break;
      }
      default:
        mooseError("Unhandled 'GeometryEnum' in 'FVThermalResistanceBC'!");
    }
  }
}

ADReal
FVThermalResistanceBC::computeQpResidual()
{
  // radiation resistance has to be solved iteratively, since we don't know the
  // surface temperature. We do know that the heat flux in the conduction layers
  // must match the heat flux in the parallel convection-radiation segment. For a
  // first guess, take the surface temperature as the average of _T and T_ambient.
  _T_surface = 0.5 * (_T[_qp] + _T_ambient);

  // total flux perpendicular to boundary
  ADReal flux;

  // resistance component representing the sum of the convection and radiation
  // resistances, in parallel
  computeParallelResistance();

  // other iteration requirements
  unsigned int iteration = 0;
  ADReal norm = 2 * _tolerance;
  ADReal T_surface_previous;

  // iterate to find the approximate surface temperature needed for evaluating the
  // radiation resistance. We only do this iteration if we have radiation transfer.
  if (_emissivity > 1e-8)
    while (norm > (_tolerance * _alpha))
    {
      T_surface_previous = _T_surface;

      // compute the flux based on the conduction part of the circuit
      flux = (_T[_qp] - _T_surface) / _conduction_resistance;

      computeParallelResistance();

      // use the flux computed from the conduction half to update T_surface
      _T_surface = flux * _parallel_resistance + _T_ambient;
      _T_surface = _alpha * _T_surface + (1 - _alpha) * T_surface_previous;
      norm = std::abs(_T_surface - T_surface_previous) / std::abs(T_surface_previous);

      if (iteration == _max_iterations)
      {
        mooseWarning("Maximum number of iterations reached in 'FVThermalResistanceBC'!");
        break;
      }
      else
        iteration += 1;
    }

  // once we have determined T_surface, we can finally evaluate the complete
  // resistance to find the overall heat flux. For Cartesian, dividing by the
  // 'inner_radius' has no effect, but it is required for correct normalization
  // for cylindrical geometries.
  flux = (_T[_qp] - _T_ambient) / (_conduction_resistance + _parallel_resistance) / _inner_radius;
  return flux;
}

void
FVThermalResistanceBC::computeParallelResistance()
{
  // compute the parallel convection and radiation resistances, assuming they
  // act on the same surface area size
  ADReal hr = _emissivity * HeatConduction::Constants::sigma *
              (_T_surface * _T_surface + _T_ambient * _T_ambient) * (_T_surface + _T_ambient);

  // for Cartesian, dividing by the 'outer_radius' has no effect, but it is
  // required for correct normalization for cylindrical geometries
  _parallel_resistance = 1.0 / (hr + _h[_qp]) / _outer_radius;
}
