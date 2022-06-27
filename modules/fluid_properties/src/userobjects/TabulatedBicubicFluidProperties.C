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

  // do we need to construct the reverse lookup p(v,e), T(v,e)?
  if (_construct_pT_from_ve)
  {
    // extreme values to set limits
    Real e1 = e_from_p_T(_pressure_min, _temperature_min);
    Real e2 = e_from_p_T(_pressure_max, _temperature_min);
    Real e3 = e_from_p_T(_pressure_min, _temperature_max);
    Real e4 = e_from_p_T(_pressure_max, _temperature_max);
    _e_min = std::min({e1, e2, e3, e4});
    _e_max = std::max({e1, e2, e3, e4});

    Real v1 = v_from_p_T(_pressure_min, _temperature_min);
    Real v2 = v_from_p_T(_pressure_max, _temperature_min);
    Real v3 = v_from_p_T(_pressure_min, _temperature_max);
    Real v4 = v_from_p_T(_pressure_max, _temperature_max);
    _v_min = std::min({v1, v2, v3, v4});
    _v_max = std::max({v1, v2, v3, v4});

    Real h1 = h_from_p_T(_pressure_min, _temperature_min);
    Real h2 = h_from_p_T(_pressure_max, _temperature_min);
    Real h3 = h_from_p_T(_pressure_min, _temperature_max);
    Real h4 = h_from_p_T(_pressure_max, _temperature_max);
    _h_min = std::min({h1, h2, h3, h4});
    _h_max = std::max({h1, h2, h3, h4});

    Real dv = (_v_max - _v_min) / ((Real)_num_v - 1);
    Real de = (_e_max - _e_min) / ((Real)_num_e - 1);
    Real dh = (_h_max - _h_min) / ((Real)_num_e - 1);

        // Create v, e, h grids for interpolation
    _specific_volume.resize(_num_v);
    for (unsigned int j = 0; j < _num_v; ++j)
      _specific_volume[j] = _v_min + j * dv;
    _internal_energy.resize(_num_e);
    for (unsigned int j = 0; j < _num_e; ++j)
      _internal_energy[j] = _e_min + j * de;
    // enthalpy & internal energy use same # grid points
    _enthalpy.resize(_num_e);
    for (unsigned int j = 0; j < _num_e; ++j)
      _enthalpy[j] = _h_min + j * dh;

    // initialize vectors for interpolation
    std::vector<std::vector<Real>> p_from_v_e(_num_v);
    std::vector<std::vector<Real>> T_from_v_e(_num_v);
    std::vector<std::vector<Real>> p_from_v_h(_num_v);
    std::vector<std::vector<Real>> T_from_v_h(_num_v);
    for (unsigned int i = 0; i < _num_v; ++i)
    {
      p_from_v_e[i].resize(_num_e);
      T_from_v_e[i].resize(_num_e);
      p_from_v_h[i].resize(_num_e);
      T_from_v_h[i].resize(_num_e);
      for (unsigned int j = 0; j < _num_e; ++j)
      {
        Real p, T;
        _fp.p_T_from_v_e(_specific_volume[i], _internal_energy[j], _p_initial_guess, _T_initial_guess, p, T);
        p_from_v_e[i][j] = p;
        T_from_v_e[i][j] = T;
        _fp.p_T_from_v_h(_specific_volume[i], _enthalpy[j], _p_initial_guess, _T_initial_guess, p, T);
        p_from_v_h[i][j] = p;
        T_from_v_h[i][j] = T;
      }
    }
    // the bicubic interpolation object are init'ed now
    _p_from_v_e_ipol =
        libmesh_make_unique<BicubicInterpolation>(_specific_volume, _internal_energy, p_from_v_e);
    _T_from_v_e_ipol =
        libmesh_make_unique<BicubicInterpolation>(_specific_volume, _internal_energy, T_from_v_e);
    _p_from_v_h_ipol =
        libmesh_make_unique<BicubicInterpolation>(_specific_volume, _enthalpy, p_from_v_h);
    _T_from_v_h_ipol =
        libmesh_make_unique<BicubicInterpolation>(_specific_volume, _enthalpy, T_from_v_h);
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
