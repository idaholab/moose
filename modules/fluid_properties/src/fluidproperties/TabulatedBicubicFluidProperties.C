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

  if (_create_direct_pT_interpolations)
  {
    _property_ipol.resize(_properties.size());
    for (const auto i : index_range(_property_ipol))
    {
      reshapeData2D(_num_p, _num_T, _properties[i], data_matrix);
      _property_ipol[i] =
          std::make_unique<BicubicInterpolation>(_pressure, _temperature, data_matrix);
    }
  }

  if (_create_direct_ve_interpolations)
  {
    _property_ve_ipol.resize(_properties_ve.size());

    for (const auto i : index_range(_property_ve_ipol))
    {
      reshapeData2D(_num_v, _num_e, _properties_ve[i], data_matrix);
      _property_ve_ipol[i] =
          std::make_unique<BicubicInterpolation>(_specific_volume, _internal_energy, data_matrix);
    }
  }

  bool conversion_succeeded = true;
  unsigned int fail_counter_ve = 0;
  unsigned int fail_counter_vh = 0;
  // Create interpolations of (p,T) from (v,e)
  if (_construct_pT_from_ve)
  {
    // Grids in specific volume and internal energy can be either linear or logarithmic
    // NOTE: this could have been called already when generating tabulated data
    createVEGridVectors();

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
          const auto old_error_behavior = _OOBBehavior;
          _OOBBehavior = SetToClosestBound;
          p_T_from_v_e(_specific_volume[i],
                       _internal_energy[j],
                       p_guess,
                       T_guess,
                       p_ve,
                       T_ve,
                       conversion_succeeded);
          _OOBBehavior = old_error_behavior;
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

  // Create interpolations of (p,T) from (v,h)
  if (_construct_pT_from_vh)
  {
    // Grids in specific volume and enthalpy can be either linear or logarithmic
    // NOTE: the specific volume grid could have been created when generating tabulated data
    createVHGridVectors();

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
          const auto old_error_behavior = _OOBBehavior;
          _OOBBehavior = SetToClosestBound;
          p_T_from_v_h(_specific_volume[i],
                       _enthalpy[j],
                       p_guess,
                       T_guess,
                       p_vh,
                       T_vh,
                       conversion_succeeded);
          _OOBBehavior = old_error_behavior;
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
                      std::to_string(number_points) + " pressure values were out of bounds\n";
  std::string T_oob = "- " + std::to_string(num_out_bounds_T) + " of " +
                      std::to_string(number_points) + " temperature values were out of bounds\n";
  std::string outcome = "The pressure and temperature values were replaced with their respective "
                        "min and max values.\n";

  // bounds are different depending on how the object is used
  std::string source = (_fp ? "from input parameters" : "from tabulation file");

  // if any of these do not exist, do not want to print them
  if (convergence_failures)
    warning_message += converge_fails;
  if (num_nans_p)
    warning_message += p_nans;
  if (num_nans_T)
    warning_message += T_nans;
  if (num_out_bounds_p)
  {
    warning_message += p_oob;
    warning_message += ("Pressure bounds " + source + ": [" + std::to_string(_pressure_min) + ", " +
                        std::to_string(_pressure_max) + "]\n");
  }
  if (num_out_bounds_T)
  {
    warning_message += T_oob;
    warning_message += ("Temperature bounds " + source + ": [" + std::to_string(_temperature_min) +
                        ", " + std::to_string(_temperature_max) + "]\n");
  }
  // print warning
  if (num_nans_p || num_nans_T || num_out_bounds_p || num_out_bounds_T || convergence_failures)
    mooseWarning(warning_message + outcome);
}
