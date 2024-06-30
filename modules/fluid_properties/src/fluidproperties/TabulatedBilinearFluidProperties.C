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
      "Fluid properties using bilinear interpolation on tabulated values provided");

  return params;
}

TabulatedBilinearFluidProperties::TabulatedBilinearFluidProperties(
    const InputParameters & parameters)
  : TabulatedFluidProperties(parameters)
{
}

void
TabulatedBilinearFluidProperties::constructInterpolation()
{
  // Construct bilinear interpolation from pre-tabulated data
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
    std::vector<std::vector<Real>> p_from_v_e(_num_v);
    std::vector<std::vector<Real>> T_from_v_e(_num_v);
    generatePTFromVE(p_from_v_e, T_from_v_e);

    // Convert from 2D vector to ColumnMajorMatrix
    ColumnMajorMatrix p_from_v_e_m(_num_v, _num_e);
    ColumnMajorMatrix T_from_v_e_m(_num_v, _num_e);

    for (unsigned int i = 0; i < _num_v; ++i)
      for (unsigned int j = 0; j < _num_e; ++j)
      {
        T_from_v_e_m(i, j) = T_from_v_e[i][j];
        p_from_v_e_m(i, j) = p_from_v_e[i][j];
      }

    // the bicubic interpolation object are init'ed now
    _p_from_v_e_ipol = libmesh_make_unique<BilinearInterpolation>(
        _specific_volume, _internal_energy, p_from_v_e_m);
    _T_from_v_e_ipol = libmesh_make_unique<BilinearInterpolation>(
        _specific_volume, _internal_energy, T_from_v_e_m);
  }

  if (_construct_pT_from_vh)
  {
    std::vector<std::vector<Real>> p_from_v_h(_num_v);
    std::vector<std::vector<Real>> T_from_v_h(_num_v);
    generatePTFromVH(p_from_v_h, T_from_v_h);

    // Convert from 2D vector to ColumnMajorMatrix
    ColumnMajorMatrix p_from_v_h_m(_num_v, _num_h);
    ColumnMajorMatrix T_from_v_h_m(_num_v, _num_h);

    for (unsigned int i = 0; i < _num_v; ++i)
      for (unsigned int j = 0; j < _num_h; ++j)
      {
        T_from_v_h_m(i, j) = T_from_v_h[i][j];
        p_from_v_h_m(i, j) = p_from_v_h[i][j];
      }

    _p_from_v_h_ipol =
        libmesh_make_unique<BilinearInterpolation>(_specific_volume, _enthalpy, p_from_v_h_m);
    _T_from_v_h_ipol =
        libmesh_make_unique<BilinearInterpolation>(_specific_volume, _enthalpy, T_from_v_h_m);
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
    // Data is flipped in BilinearInterpolation because of the column major indexing
    for (unsigned int i = 0; i < nrow; ++i)
      for (unsigned int j = 0; j < ncol; ++j)
        mat(i, j) = vec[j * nrow + i];
  }
}
