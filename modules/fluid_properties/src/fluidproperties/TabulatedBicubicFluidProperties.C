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

  // Create specific volume (v) grid
  if (_construct_pT_from_ve || _construct_pT_from_vh)
  {
    std::vector<std::vector<Real>> p_from_v_e(_num_v);
    std::vector<std::vector<Real>> T_from_v_e(_num_v);
    generatePTFromVE(p_from_v_e, T_from_v_e);

    // the bicubic interpolation object are init'ed now
    _p_from_v_e_ipol =
        std::make_unique<BicubicInterpolation>(_specific_volume, _internal_energy, p_from_v_e);
    _T_from_v_e_ipol =
        std::make_unique<BicubicInterpolation>(_specific_volume, _internal_energy, T_from_v_e);
  }

  if (_construct_pT_from_vh)
  {
    std::vector<std::vector<Real>> p_from_v_h(_num_v);
    std::vector<std::vector<Real>> T_from_v_h(_num_v);
    generatePTFromVH(p_from_v_h, T_from_v_h);

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
