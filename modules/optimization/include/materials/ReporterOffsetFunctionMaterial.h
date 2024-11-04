//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "Function.h"
#include "ReporterInterface.h"

template <bool>
class ReporterOffsetFunctionMaterialTempl;
typedef ReporterOffsetFunctionMaterialTempl<false> ReporterOffsetFunctionMaterial;
typedef ReporterOffsetFunctionMaterialTempl<true> ADReporterOffsetFunctionMaterial;

/**
 * This class defines a material with an associated offset function.
 * It is designed to be used in conjunction with a reporter, allowing
 * for flexible and customizable material properties in a simulation.
 */
template <bool is_ad>
class ReporterOffsetFunctionMaterialTempl : public Material, public ReporterInterface
{
public:
  static InputParameters validParams();

  ReporterOffsetFunctionMaterialTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Base name of the material system
  const std::string _prop_name;

  /// Misfit value at each quadrature point
  GenericMaterialProperty<Real, is_ad> & _material;

  /// bool if data format read in is points
  const bool _read_in_points;
  /**
   * Reporter offset locations for function
   */
  // @{
  /// x coordinate
  const std::vector<Real> & _coordx;
  /// y coordinate
  const std::vector<Real> & _coordy;
  ///z coordinate
  const std::vector<Real> & _coordz;
  ///xyz point
  const std::vector<Point> & _points;
  // @}

  /// The function being used for evaluation
  const Function & _func;
  /**
   * Calculates the value of the function at the given offset point.
   * @param point_offset The offset point to shift the function evaluation by.
   * @return The value of the function at the shifted location.
   */
  Real computeOffsetFunction(const Point & point_offset);

private:
  /// convenience vectors (these are not const because reporters can change their size)
  std::vector<Real> _zeros_vec;
  std::vector<Point> _zeros_pts;
};
