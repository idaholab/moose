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
{}

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

  // do we need to construct the reverse lookup p(v,e), T(v,e)?
  if (_construct_pT_from_ve)
  {
    // extreme values to set limits
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

    if (_construct_from_ve)
    {
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
      std::vector<std::vector<Real>> p_from_v_e(_num_v);
      std::vector<std::vector<Real>> T_from_v_e(_num_v);

      unsigned int num_p_nans_ve = 0, num_T_nans_ve = 0, num_p_out_bounds_ve = 0, num_T_out_bounds_ve = 0;

      for (unsigned int i = 0; i < _num_v; ++i)
      {
        p_from_v_e[i].resize(_num_e);
        T_from_v_e[i].resize(_num_e);
        for (unsigned int j = 0; j < _num_e; ++j)
        {
          Real p_ve, T_ve;
          _fp.p_T_from_v_e(_specific_volume[i], _internal_energy[j], _p_initial_guess, _T_initial_guess, p_ve, T_ve);

          // std::cout << "sv & ie " << _specific_volume[i] << " " << _internal_energy[j] << std::endl;
          // std::cout << "from_p_T " << _fp.v_from_p_T(p, T) << " " << _fp.e_from_p_T(p, T) << std::endl;

          /// replace nan values with pmax or pmin
          if (std::isnan(p_ve))
          {
            std::cout << "i = " << i << "j = " << j << "p_ve = " << p_ve << std::endl;
            if (_specific_volume[i] > ((_v_min + _v_max) / 2) )
              p_ve = _pressure_min;
            else if (_specific_volume[i] < ((_v_min + _v_max) / 2) )
              p_ve = _pressure_max;
            num_p_nans_ve++;
          }
          if (std::isnan(T_ve))
          {
            std::cout << "i = " << i << "j = " << j << "T_ve = " << T_ve << std::endl;
            if (_specific_volume[i] > ((_v_min + _v_max) / 2) )
              T_ve = _temperature_max;
            else if (_specific_volume[i] < ((_v_min + _v_max) / 2) )
              T_ve = _temperature_min;
            num_T_nans_ve++;
          }
          //// replace out of bounds pressure values with pmax or pmin
          if (p_ve < _pressure_min)
          {
            p_ve = _pressure_min;
            num_p_out_bounds_ve++;
          }
          else if (p_ve > _pressure_max)
          {
            p_ve = _pressure_max;
            num_p_out_bounds_ve++;
          }
          //// replace out of bounds temperature values with Tmax or Tmin
          if (T_ve < _temperature_min)
          {
            T_ve = _temperature_min;
            num_T_out_bounds_ve++;
          }
          else if (T_ve > _temperature_max)
          {
            T_ve = _temperature_max;
            num_T_out_bounds_ve++;
          }
          p_from_v_e[i][j] = p_ve;
          T_from_v_e[i][j] = T_ve;
        }
      }
      if (num_p_nans_ve > 0)
        mooseWarning("while creating v,e interpolation tables, ",
                     num_p_nans_ve,
                     " NaNs were computed for pressure during v,e to p,T inversions.");
      if (num_p_out_bounds_ve > 0)
        mooseWarning("while creating v,e interpolation tables, ",
                     num_p_out_bounds_ve,
                     " computed pressure values out of user defined range for _num_p out of bounds");
      if (num_T_nans_ve > 0)
        mooseWarning("while creating v,e interpolation tables, ",
                     num_T_nans_ve,
                     " NaNs were computed for temperature during v,e to p,T inversions.");
      if (num_T_out_bounds_ve > 0)
        mooseWarning("while creating v,e interpolation tables, ",
                     num_T_out_bounds_ve,
                     " computed temperature values out of user defined range for out of bounds");
                     // the bicubic interpolation object are init'ed now
      _p_from_v_e_ipol =
         libmesh_make_unique<BicubicInterpolation>(_specific_volume, _internal_energy, p_from_v_e);
      _T_from_v_e_ipol =
         libmesh_make_unique<BicubicInterpolation>(_specific_volume, _internal_energy, T_from_v_e);
    }

    if (_construct_from_vh)
    {
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
      std::vector<std::vector<Real>> p_from_v_h(_num_v);
      std::vector<std::vector<Real>> T_from_v_h(_num_v);

      unsigned int num_p_nans_vh = 0, num_T_nans_vh = 0, num_p_out_bounds_vh = 0, num_T_out_bounds_vh = 0;

      for (unsigned int i = 0; i < _num_v; ++i)
      {
        p_from_v_h[i].resize(_num_e);
        T_from_v_h[i].resize(_num_e);
        for (unsigned int j = 0; j < _num_e; ++j)
        {
          Real p_vh, T_vh;
          _fp.p_T_from_v_h(_specific_volume[i], _enthalpy[j], _p_initial_guess, _T_initial_guess, p_vh, T_vh);
          // std::cout << "h " << p << " " << T << std::endl;

          //// replace nan values with pmax or pmin
          if (std::isnan(p_vh))
          {
            std::cout << "i = " << i << "j = " << j << "p_vh = " << p_vh << std::endl;
            if (_specific_volume[i] > ((_v_min + _v_max) / 2) )
              p_vh = _pressure_min;
            else if (_specific_volume[i] < ((_v_min + _v_max) / 2) )
              p_vh = _pressure_max;
            num_p_nans_vh++;
          }
          if (std::isnan(T_vh))
          {
            std::cout << "i = " << i << "j = " << j << "T_vh = " << T_vh << std::endl;
            if (_specific_volume[i] > ((_v_min + _v_max) / 2) )
              T_vh = _temperature_max;
            else if (_specific_volume[i] < ((_v_min + _v_max) / 2) )
              T_vh = _temperature_min;
            num_T_nans_vh++;
          }
          //// replace out of bounds pressure values with pmax or pmin
          if (p_vh < _pressure_min)
          {
            p_vh = _pressure_min;
            num_p_out_bounds_vh++;
          }
          else if (p_vh > _pressure_max)
          {
            p_vh = _pressure_max;
            num_p_out_bounds_vh++;
          }
          //// replace out of bounds temperature values with Tmax or Tmin
          if (T_vh < _temperature_min)
          {
            T_vh = _temperature_min;
            num_T_out_bounds_vh++;
          }
          else if (T_vh > _temperature_max)
          {
            T_vh = _temperature_max;
            num_T_out_bounds_vh++;
          }
          p_from_v_h[i][j] = p_vh;
          T_from_v_h[i][j] = T_vh;
        }
      }
      if (num_p_nans_vh > 0)
        mooseWarning("while creating v,h interpolation tables, ",
                     num_p_nans_vh,
                     " NaNs were computed for pressure during v,h to p,T inversions.");
      if (num_p_out_bounds_vh > 0)
        mooseWarning("while creating v,h interpolation tables, ",
                     num_p_out_bounds_vh,
                     " computed pressure values out of user defined range for _num_p out of bounds");
      if (num_T_nans_vh > 0)
        mooseWarning("while creating v,h interpolation tables, ",
                     num_T_nans_vh,
                     " NaNs were computed for temperature during v,h to p,T inversions.");
      if (num_T_out_bounds_vh > 0)
        mooseWarning("while creating v,h interpolation tables, ",
                     num_T_out_bounds_vh,
                     " computed temperature values out of user defined range for out of bounds");
      _p_from_v_h_ipol =
         libmesh_make_unique<BicubicInterpolation>(_specific_volume, _enthalpy, p_from_v_h);
      _T_from_v_h_ipol =
         libmesh_make_unique<BicubicInterpolation>(_specific_volume, _enthalpy, T_from_v_h);
    }
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
