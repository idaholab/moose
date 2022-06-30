//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TabulatedBilinearFluidProperties.h"
#include "BilinearInterpolation.h"
#include "MooseUtils.h"
#include "Conversion.h"
#include "KDTree.h"

// C++ includes
#include <fstream>
#include <ctime>

registerMooseObject("FluidPropertiesApp", TabulatedBilinearFluidProperties);

InputParameters
TabulatedBilinearFluidProperties::validParams()
{
  InputParameters params = TabulatedFluidProperties::validParams();
  params.addClassDescription(
      "Fluid properties using bicubic interpolation on tabulated values provided");

  return params;
}

TabulatedBilinearFluidProperties::TabulatedBilinearFluidProperties(const InputParameters & parameters)
  : TabulatedFluidProperties(parameters)
{}

void
TabulatedBilinearFluidProperties::constructInterpolation()
{
  // Construct bilinear interpolants from tabulated data
  ColumnMajorMatrix data_matrix(_num_p, _num_T);
  _property_ipol.resize(_properties.size());

  for (std::size_t i = 0; i < _property_ipol.size(); ++i)
  {
    reshapeData2D(_num_p, _num_T, _properties[i], data_matrix);
    _property_ipol[i] =
        std::make_unique<BilinearInterpolation>(_pressure, _temperature, data_matrix);
  }

  // Create specific volume (v) grid
  if (_construct_pT_from_ve || _construct_pT_from_vh)
  {
    // extreme values of specific volume for the grid bounds
    Real v1 = v_from_p_T(_pressure_min, _temperature_min);
    Real v2 = v_from_p_T(_pressure_max, _temperature_min);
    Real v3 = v_from_p_T(_pressure_min, _temperature_max);
    Real v4 = v_from_p_T(_pressure_max, _temperature_max);
    _v_min = std::min({v1, v2, v3, v4});
    _v_max = std::max({v1, v2, v3, v4});
    Real dv = (_v_max - _v_min) / ((Real)_num_v - 1);

    // Create v grid for interpolation
    _specific_volume.resize(_num_v);
    for (unsigned int j = 0; j < _num_v; ++j)
      _specific_volume[j] = _v_min + j * dv;

    if (_construct_pT_from_vh)
    {
      // extreme values of internal energy for the grid bounds
      Real e1 = e_from_p_T(_pressure_min, _temperature_min);
      Real e2 = e_from_p_T(_pressure_max, _temperature_min);
      Real e3 = e_from_p_T(_pressure_min, _temperature_max);
      Real e4 = e_from_p_T(_pressure_max, _temperature_max);
      _e_min = std::min({e1, e2, e3, e4});
      _e_max = std::max({e1, e2, e3, e4});
      Real de = (_e_max - _e_min) / ((Real)_num_e - 1);

      // Create e grid for interpolation
      _internal_energy.resize(_num_e);
      for (unsigned int j = 0; j < _num_e; ++j)
        _internal_energy[j] = _e_min + j * de;

      // initialize vectors for interpolation
      ColumnMajorMatrix p_from_v_e(_num_v,_num_e);
      ColumnMajorMatrix T_from_v_e(_num_v,_num_e);

      unsigned int num_p_nans_ve = 0, num_T_nans_ve = 0, num_p_out_bounds_ve = 0, num_T_out_bounds_ve = 0;

      for (unsigned int i = 0; i < _num_v; ++i)
      {
        for (unsigned int j = 0; j < _num_e; ++j)
        {
          Real p_ve, T_ve;
          _fp->p_T_from_v_e(_specific_volume[i],
                            _internal_energy[j],
                            _p_initial_guess,
                            _T_initial_guess,
                            p_ve,
                            T_ve);

          /// check for NaNs in p interpolation
          checkNaNs(_pressure_min, _pressure_max, i, p_ve, num_p_nans_ve);
          /// check for NaNs in p interpolation
          checkNaNs( _temperature_min, _temperature_max, i, T_ve,num_T_nans_ve);

          /// replace out of bounds pressure values with pmax or pmin
          checkOutofBounds(_pressure_min, _pressure_max, p_ve, num_p_out_bounds_ve);
          /// replace out of bounds temperature values with Tmax or Tmin
          checkOutofBounds(_temperature_min, _temperature_max, T_ve, num_T_out_bounds_ve);

          p_from_v_e(i, j) = p_ve;
          T_from_v_e(i, j) = T_ve;
        }
      }

      outputWarnings(num_p_nans_ve, num_p_out_bounds_ve, "(v,e)", "pressure", _num_e*_num_v);
      outputWarnings(num_T_nans_ve, num_T_out_bounds_ve, "(v,e)", "temperature", _num_e*_num_v);

                     // the bicubic interpolation object are init'ed now
      _p_from_v_e_ipol =
         libmesh_make_unique<BilinearInterpolation>(_specific_volume, _internal_energy, p_from_v_e);
      _T_from_v_e_ipol =
         libmesh_make_unique<BilinearInterpolation>(_specific_volume, _internal_energy, T_from_v_e);
    }

    if (_construct_pT_from_vh)
    {
      // extreme values of enthalpy for the grid bounds
      Real h1 = h_from_p_T(_pressure_min, _temperature_min);
      Real h2 = h_from_p_T(_pressure_max, _temperature_min);
      Real h3 = h_from_p_T(_pressure_min, _temperature_max);
      Real h4 = h_from_p_T(_pressure_max, _temperature_max);
      _h_min = std::min({h1, h2, h3, h4});
      _h_max = std::max({h1, h2, h3, h4});
      Real dh = (_h_max - _h_min) / ((Real)_num_e - 1);

      // Create h grid for interpolation
      // enthalpy & internal energy use same # grid points
      _enthalpy.resize(_num_e);
      for (unsigned int j = 0; j < _num_e; ++j)
        _enthalpy[j] = _h_min + j * dh;

      // initialize vectors for interpolation
      ColumnMajorMatrix p_from_v_h(_num_v,_num_e);
      ColumnMajorMatrix T_from_v_h(_num_v,_num_e);

      unsigned int num_p_nans_vh = 0, num_T_nans_vh = 0, num_p_out_bounds_vh = 0, num_T_out_bounds_vh = 0;

      for (unsigned int i = 0; i < _num_v; ++i)
      {
        for (unsigned int j = 0; j < _num_e; ++j)
        {
          Real p_vh, T_vh;
<<<<<<< HEAD
          _fp.p_T_from_v_h(_specific_volume[i], _enthalpy[j], _p_initial_guess, _T_initial_guess, p_vh, T_vh);
=======
          _fp->p_T_from_v_h(
              _specific_volume[i], _enthalpy[j], _p_initial_guess, _T_initial_guess, p_vh, T_vh);
>>>>>>> 9fde525054 (fixup! Added checkNaNs routine to TabBicubicFP and TabBilinearFP. If nans exist, set variable to be constant as min or max, depending on where nans exist. Also added a checkOutofBounds routine to set variable to be constant at min or max depending on where variable is out of bounds. Added T_from_p_h routine using Newton Method. Added s_from_p_h routine which uses T_from_p_h, then finds s_from_p_T. Added feature to TabBicubicFP and TabBilinearFP which allows user to choose to generate (p,T) from both (v,e) and (v,h) or choose one or the other. Added routine to TFP to check the initial guesses for p and T that are used in Newton's Mehotd. If these guesses are outside the range for max/min of p and T, routine produces an error and informs the user. Ref #20101)

          /// check for NaNs in p interpolation
          checkNaNs(_pressure_min, _pressure_max, i, p_vh, num_p_nans_vh);
          /// check for NaNs in p interpolation
          checkNaNs( _temperature_min, _temperature_max, i, T_vh, num_T_nans_vh);

          /// replace out of bounds pressure values with pmax or pmin
          checkOutofBounds(_pressure_min, _pressure_max, p_vh, num_p_out_bounds_vh);
          /// replace out of bounds temperature values with Tmax or Tmin
          checkOutofBounds(_temperature_min, _temperature_max, T_vh, num_T_out_bounds_vh);
          p_from_v_h(i, j) = p_vh;
          T_from_v_h(i, j) = T_vh;
        }
      }

      outputWarnings(num_p_nans_vh, num_p_out_bounds_vh, "(v,h)", "pressure", _num_e*_num_v);
      outputWarnings(num_T_nans_vh, num_T_out_bounds_vh, "(v,h)", "temperature", _num_e*_num_v);

      _p_from_v_h_ipol =
         libmesh_make_unique<BilinearInterpolation>(_specific_volume, _enthalpy, p_from_v_h);
      _T_from_v_h_ipol =
         libmesh_make_unique<BilinearInterpolation>(_specific_volume, _enthalpy, T_from_v_h);
    }
  }
}

void
TabulatedBilinearFluidProperties::reshapeData2D(unsigned int nrow,
                                        unsigned int ncol,
                                        const std::vector<Real> & vec,
                                        ColumnMajorMatrix & mat)
{
  if (!vec.empty())
  {
    for (unsigned int i = 0; i < nrow; ++i)
      for (unsigned int j = 0; j < ncol; ++j)
        mat(i,j) = vec[i * ncol + j];
  }
}

void
TabulatedBilinearFluidProperties::checkNaNs(Real min, Real max, unsigned int i, Real & variable, unsigned int & num_nans)
{
  /// replace nan values with pmax or pmin
  if (std::isnan(variable))
  {
    if (_specific_volume[i] > ((_v_min + _v_max) / 2) )
      variable = min;
    else if (_specific_volume[i] < ((_v_min + _v_max) / 2) )
      variable = max;
    num_nans++;
  }
}

void
TabulatedBilinearFluidProperties::checkOutofBounds(Real min, Real max, Real & variable, unsigned int & num_out_bounds)
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
TabulatedBilinearFluidProperties::outputWarnings(Real num_nans, Real num_out_bounds, std::string variable_set, std::string p_or_T, unsigned int number_points)
{
  if (num_nans)
    mooseWarning("while creating ", variable_set, " interpolation tables, ",
                 num_nans,
                 " NaNs were computed for ", p_or_T , " during ", variable_set, " to p,T inversions.");
  if (num_out_bounds)
    mooseWarning("while creating ", variable_set, " interpolation tables, ",
                 num_out_bounds,
                 " of ",
                 number_points,
                 " computed ", p_or_T, " values were out of user defined range.");
  if (num_nans || num_out_bounds)
    mooseWarning("NaNs and out-of-bounds values were replaced with user-defined range, forcing a "
                 "constant value for the interpolated quantities in that domain");
}
