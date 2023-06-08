//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TabulatedBicubicFluidProperties.h"
#include "BicubicInterpolation.h"
#include "MooseUtils.h"
#include "Conversion.h"
#include "KDTree.h"

// C++ includes
#include <fstream>
#include <ctime>

registerMooseObject("FluidPropertiesApp", TabulatedBicubicFluidProperties);
registerMooseObjectRenamed("FluidPropertiesApp",
                           TabulatedFluidProperties,
                           "01/01/2023 00:00",
                           TabulatedBicubicFluidProperties);

InputParameters
TabulatedBicubicFluidProperties::validParams()
{
  InputParameters params = TabulatedFluidProperties::validParams();
  params.addClassDescription(
      "Fluid properties using bicubic interpolation on tabulated values provided");
  return params;
}

TabulatedBicubicFluidProperties::TabulatedBicubicFluidProperties(const InputParameters & parameters)
  : TabulatedFluidProperties(parameters)
{
}

void
TabulatedBicubicFluidProperties::constructInterpolation()
{
  // Construct bicubic interpolants from tabulated data
  std::vector<std::vector<Real>> data_matrix;
  _property_ipol.resize(_properties.size());

  for (std::size_t i = 0; i < _property_ipol.size(); ++i)
  {
    reshapeData2D(_num_p, _num_T, _properties[i], data_matrix);
    _property_ipol[i] =
        std::make_unique<BicubicInterpolation>(_pressure, _temperature, data_matrix);
  }

  bool conversion_succeeded = true;
  unsigned int fail_counter_ve = 0;
  unsigned int fail_counter_vh = 0;
  // Create specific volume (v) grid
  if (_construct_pT_from_ve || _construct_pT_from_vh)
  {
    if (_fp)
    {
      // extreme values of specific volume for the grid bounds
      Real v1 = v_from_p_T(_pressure_min, _temperature_min);
      Real v2 = v_from_p_T(_pressure_max, _temperature_min);
      Real v3 = v_from_p_T(_pressure_min, _temperature_max);
      Real v4 = v_from_p_T(_pressure_max, _temperature_max);
      _v_min = std::min({v1, v2, v3, v4});
      _v_max = std::max({v1, v2, v3, v4});
    }
    // if csv exists, get max and min values from csv file
    else
    {
      Real rho_max =
          *max_element(_properties[_density_idx].begin(), _properties[_density_idx].end());
      Real rho_min =
          *min_element(_properties[_density_idx].begin(), _properties[_density_idx].end());
      _v_max = 1 / rho_min;
      _v_min = 1 / rho_max;
    }

    // Create v grid for interpolation
    _specific_volume.resize(_num_v);
    if (_log_space_v)
    {
      // incrementing the exponent linearly will yield a log-spaced grid after taking the value to
      // the power of 10
      Real dv = (std::log10(_v_max) - std::log10(_v_min)) / ((Real)_num_v - 1);
      Real log_v_min = std::log10(_v_min);
      for (unsigned int j = 0; j < _num_v; ++j)
        _specific_volume[j] = std::pow(10, log_v_min + j * dv);
    }
    else
    {
      Real dv = (_v_max - _v_min) / ((Real)_num_v - 1);
      for (unsigned int j = 0; j < _num_v; ++j)
        _specific_volume[j] = _v_min + j * dv;
    }
  }

  if (_construct_pT_from_ve)
  {
    if (_fp)
    {
      // extreme values of internal energy for the grid bounds
      Real e1 = e_from_p_T(_pressure_min, _temperature_min);
      Real e2 = e_from_p_T(_pressure_max, _temperature_min);
      Real e3 = e_from_p_T(_pressure_min, _temperature_max);
      Real e4 = e_from_p_T(_pressure_max, _temperature_max);
      _e_min = std::min({e1, e2, e3, e4});
      _e_max = std::max({e1, e2, e3, e4});
    }
    // if csv exists, get max and min values from csv file
    else
    {
      _e_max = *max_element(_properties[_internal_energy_idx].begin(),
                            _properties[_internal_energy_idx].end());
      _e_min = *min_element(_properties[_internal_energy_idx].begin(),
                            _properties[_internal_energy_idx].end());
    }
    Real de = (_e_max - _e_min) / ((Real)_num_e - 1);

    // Create e grid for interpolation
    _internal_energy.resize(_num_e);
    for (unsigned int j = 0; j < _num_e; ++j)
      _internal_energy[j] = _e_min + j * de;

    // initialize vectors for interpolation
    std::vector<std::vector<Real>> p_from_v_e(_num_v);
    std::vector<std::vector<Real>> T_from_v_e(_num_v);

    unsigned int num_p_nans_ve = 0, num_T_nans_ve = 0, num_p_out_bounds_ve = 0,
                 num_T_out_bounds_ve = 0;

    for (unsigned int i = 0; i < _num_v; ++i)
    {
      Real p_guess = _p_initial_guess;
      Real T_guess = _T_initial_guess;
      p_from_v_e[i].resize(_num_e);
      T_from_v_e[i].resize(_num_e);
      for (unsigned int j = 0; j < _num_e; ++j)
      {
        Real p_ve, T_ve;
        // Using an input fluid property instead of the tabulation will get more exact inversions
        if (_fp)
          _fp->p_T_from_v_e(_specific_volume[i],
                            _internal_energy[j],
                            _p_initial_guess,
                            _T_initial_guess,
                            p_ve,
                            T_ve,
                            conversion_succeeded);
        else
        {
          // The inversion may step outside of the domain of definition of the interpolations,
          // which are restricted to the range of the input CSV file
          const bool old_error_behavior = _error_on_out_of_bounds;
          _error_on_out_of_bounds = false;
          p_T_from_v_e(_specific_volume[i],
                       _internal_energy[j],
                       p_guess,
                       T_guess,
                       p_ve,
                       T_ve,
                       conversion_succeeded);
          _error_on_out_of_bounds = old_error_behavior;
          // track number of times convergence failed
          if (!conversion_succeeded)
            ++fail_counter_ve;
        }

        // check for NaNs in Newton Method
        checkNaNs(_pressure_min,
                  _pressure_max,
                  _temperature_min,
                  _temperature_max,
                  i,
                  p_ve,
                  T_ve,
                  num_p_nans_ve,
                  num_T_nans_ve);

        // replace out of bounds pressure values with pmax or pmin
        checkOutofBounds(_pressure_min, _pressure_max, p_ve, num_p_out_bounds_ve);
        // replace out of bounds temperature values with Tmax or Tmin
        checkOutofBounds(_temperature_min, _temperature_max, T_ve, num_T_out_bounds_ve);

        p_from_v_e[i][j] = p_ve;
        T_from_v_e[i][j] = T_ve;

        p_guess = p_ve;
        T_guess = T_ve;
      }
    }
    // output warning if nans or values out of bounds
    outputWarnings(num_p_nans_ve,
                   num_T_nans_ve,
                   num_p_out_bounds_ve,
                   num_T_out_bounds_ve,
                   fail_counter_ve,
                   _num_e * _num_v,
                   "(v,e)");

    // the bicubic interpolation object are init'ed now
    _p_from_v_e_ipol =
        std::make_unique<BicubicInterpolation>(_specific_volume, _internal_energy, p_from_v_e);
    _T_from_v_e_ipol =
        std::make_unique<BicubicInterpolation>(_specific_volume, _internal_energy, T_from_v_e);
  }

  if (_construct_pT_from_vh)
  {
    if (_fp)
    {
      // extreme values of enthalpy for the grid bounds
      Real h1 = h_from_p_T(_pressure_min, _temperature_min);
      Real h2 = h_from_p_T(_pressure_max, _temperature_min);
      Real h3 = h_from_p_T(_pressure_min, _temperature_max);
      Real h4 = h_from_p_T(_pressure_max, _temperature_max);
      _h_min = std::min({h1, h2, h3, h4});
      _h_max = std::max({h1, h2, h3, h4});
    }
    // if csv exists, get max and min values from csv file
    else
    {
      _h_max = *max_element(_properties[_enthalpy_idx].begin(), _properties[_enthalpy_idx].end());
      _h_min = *min_element(_properties[_enthalpy_idx].begin(), _properties[_enthalpy_idx].end());
    }
    Real dh = (_h_max - _h_min) / ((Real)_num_e - 1);

    // Create h grid for interpolation
    // enthalpy & internal energy use same # grid points
    _enthalpy.resize(_num_e);
    for (unsigned int j = 0; j < _num_e; ++j)
      _enthalpy[j] = _h_min + j * dh;

    // initialize vectors for interpolation
    std::vector<std::vector<Real>> p_from_v_h(_num_v);
    std::vector<std::vector<Real>> T_from_v_h(_num_v);

    unsigned int num_p_nans_vh = 0, num_T_nans_vh = 0, num_p_out_bounds_vh = 0,
                 num_T_out_bounds_vh = 0;

    for (unsigned int i = 0; i < _num_v; ++i)
    {
      Real p_guess = _p_initial_guess;
      Real T_guess = _T_initial_guess;
      p_from_v_h[i].resize(_num_e);
      T_from_v_h[i].resize(_num_e);
      for (unsigned int j = 0; j < _num_e; ++j)
      {
        Real p_vh, T_vh;
        // Using an input fluid property instead of the tabulation will get more exact inversions
        if (_fp)
          _fp->p_T_from_v_h(_specific_volume[i],
                            _enthalpy[j],
                            _p_initial_guess,
                            _T_initial_guess,
                            p_vh,
                            T_vh,
                            conversion_succeeded);
        else
        {
          // The inversion may step outside of the domain of definition of the interpolations,
          // which are restricted to the range of the input CSV file
          const bool old_error_behavior = _error_on_out_of_bounds;
          _error_on_out_of_bounds = false;
          p_T_from_v_h(_specific_volume[i],
                       _enthalpy[j],
                       p_guess,
                       T_guess,
                       p_vh,
                       T_vh,
                       conversion_succeeded);
          _error_on_out_of_bounds = old_error_behavior;
          // track number of times convergence failed
          if (!conversion_succeeded)
            ++fail_counter_vh;
        }

        // check for NaNs in Newton Method
        checkNaNs(_pressure_min,
                  _pressure_max,
                  _temperature_min,
                  _temperature_max,
                  i,
                  p_vh,
                  T_vh,
                  num_p_nans_vh,
                  num_T_nans_vh);

        // replace out of bounds pressure values with pmax or pmin
        checkOutofBounds(_pressure_min, _pressure_max, p_vh, num_p_out_bounds_vh);
        // replace out of bounds temperature values with Tmax or Tmin
        checkOutofBounds(_temperature_min, _temperature_max, T_vh, num_T_out_bounds_vh);

        p_from_v_h[i][j] = p_vh;
        T_from_v_h[i][j] = T_vh;

        p_guess = p_vh;
        T_guess = T_vh;
      }
    }
    // output warnings if nans our values out of bounds
    outputWarnings(num_p_nans_vh,
                   num_T_nans_vh,
                   num_p_out_bounds_vh,
                   num_T_out_bounds_vh,
                   fail_counter_vh,
                   _num_e * _num_v,
                   "(v,h)");

    // the bicubic interpolation object are init'ed now
    _p_from_v_h_ipol =
        std::make_unique<BicubicInterpolation>(_specific_volume, _enthalpy, p_from_v_h);
    _T_from_v_h_ipol =
        std::make_unique<BicubicInterpolation>(_specific_volume, _enthalpy, T_from_v_h);
  }
}

void
TabulatedBicubicFluidProperties::reshapeData2D(unsigned int nrow,
                                               unsigned int ncol,
                                               const std::vector<Real> & vec,
                                               std::vector<std::vector<Real>> & mat)
{
  if (!vec.empty())
  {
    mat.resize(nrow);
    for (unsigned int i = 0; i < nrow; ++i)
      mat[i].resize(ncol);

    for (unsigned int i = 0; i < nrow; ++i)
      for (unsigned int j = 0; j < ncol; ++j)
        mat[i][j] = vec[i * ncol + j];
  }
}

void
TabulatedBicubicFluidProperties::checkNaNs(Real min_1,
                                           Real max_1,
                                           Real min_2,
                                           Real max_2,
                                           unsigned int i,
                                           Real & variable_1,
                                           Real & variable_2,
                                           unsigned int & num_nans_1,
                                           unsigned int & num_nans_2)
{
  // replace nan values with pmax or pmin
  if (std::isnan(variable_1))
  {
    if (_specific_volume[i] > ((_v_min + _v_max) / 2))
      variable_1 = min_1;
    else if (_specific_volume[i] < ((_v_min + _v_max) / 2))
      variable_1 = max_1;
    num_nans_1++;
  }
  // replace nan values with Tmax or Tmin
  if (std::isnan(variable_2))
  {
    if (_specific_volume[i] > ((_v_min + _v_max) / 2))
      variable_2 = max_2;
    else if (_specific_volume[i] < ((_v_min + _v_max) / 2))
      variable_2 = min_2;
    num_nans_2++;
  }
}

void
TabulatedBicubicFluidProperties::checkOutofBounds(Real min,
                                                  Real max,
                                                  Real & variable,
                                                  unsigned int & num_out_bounds)
{
  if (variable < min)
  {
    variable = min;
    num_out_bounds++;
  }
  else if (variable > max)
  {
    variable = max;
    num_out_bounds++;
  }
}

void
TabulatedBicubicFluidProperties::outputWarnings(unsigned int num_nans_p,
                                                unsigned int num_nans_T,
                                                unsigned int num_out_bounds_p,
                                                unsigned int num_out_bounds_T,
                                                unsigned int convergence_failures,
                                                unsigned int number_points,
                                                std::string variable_set)
{
  // make string variables before mooseWarning
  std::string while_creating =
      "While creating (p,T) from " + variable_set + " interpolation tables,\n";
  std::string warning_message = while_creating;
  std::string converge_fails = "Inversion to (p,T) from " + variable_set + " failed " +
                               std::to_string(convergence_failures) + " times\n";
  std::string p_nans = "- " + std::to_string(num_nans_p) + " nans generated out of " +
                       std::to_string(number_points) + " points for pressure\n";
  std::string T_nans = "- " + std::to_string(num_nans_T) + " nans generated out of " +
                       std::to_string(number_points) + " points for temperature\n";
  std::string p_oob = "- " + std::to_string(num_out_bounds_p) + " of " +
                      std::to_string(number_points) +
                      " pressure values were out of user defined bounds\n";
  std::string T_oob = "- " + std::to_string(num_out_bounds_T) + " of " +
                      std::to_string(number_points) +
                      " temperature values were out of user defined bounds\n";
  std::string outcome = "The pressure and temperature values were replaced with their respective "
                        "user-defined min and max values.\n";
  // if any of these do not exist, do not want to print them
  if (convergence_failures)
    warning_message += converge_fails;
  if (num_nans_p)
    warning_message += p_nans;
  if (num_nans_T)
    warning_message += T_nans;
  if (num_out_bounds_p)
    warning_message += p_oob;
  if (num_out_bounds_T)
    warning_message += T_oob;
  // print warning
  if (num_nans_p || num_nans_T || num_out_bounds_p || num_out_bounds_T || convergence_failures)
    mooseWarning(warning_message + outcome);
}
