//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ReporterOffsetFunctionMaterial.h"

template <bool>
class MisfitReporterOffsetFunctionMaterialTempl;
typedef MisfitReporterOffsetFunctionMaterialTempl<false> MisfitReporterOffsetFunctionMaterial;
typedef MisfitReporterOffsetFunctionMaterialTempl<true> ADMisfitReporterOffsetFunctionMaterial;

/**
 * This class creates a misfit and misfit gradient material that can be used for
 * optimizing measurements weighted by offset functions.
 */
template <bool is_ad>
class MisfitReporterOffsetFunctionMaterialTempl : public ReporterOffsetFunctionMaterialTempl<is_ad>
{
public:
  static InputParameters validParams();

  MisfitReporterOffsetFunctionMaterialTempl<is_ad>(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Simulation variable
  const GenericVariableValue<is_ad> & _sim_var;

  /// Gradient of misfit with respect to the simulation variable
  GenericMaterialProperty<Real, is_ad> & _mat_prop_gradient;

  /// values at each xyz coordinate
  const std::vector<Real> & _measurement_values;

  Real computeWeighting(const Point & point_shift);

  using ReporterOffsetFunctionMaterialTempl<is_ad>::_prop_name;
  using ReporterOffsetFunctionMaterialTempl<is_ad>::_qp;
  using ReporterOffsetFunctionMaterialTempl<is_ad>::_material;
  using ReporterOffsetFunctionMaterialTempl<is_ad>::_points;
  using ReporterOffsetFunctionMaterialTempl<is_ad>::_read_in_points;
  using ReporterOffsetFunctionMaterialTempl<is_ad>::_coordx;
  using ReporterOffsetFunctionMaterialTempl<is_ad>::_coordy;
  using ReporterOffsetFunctionMaterialTempl<is_ad>::_coordz;
  using ReporterOffsetFunctionMaterialTempl<is_ad>::computeOffsetFunction;
};
