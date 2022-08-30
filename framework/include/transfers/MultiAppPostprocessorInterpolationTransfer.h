//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MultiAppTransfer.h"

/**
 * Transfers from postprocessors in child apps of a MultiApp in different locations in parent app
 * mesh, interpolates the values between the app locations then fills a field variable
 * in the parent app system.
 */
class MultiAppPostprocessorInterpolationTransfer : public MultiAppTransfer
{
public:
  static InputParameters validParams();

  MultiAppPostprocessorInterpolationTransfer(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /// Postprocessor in the child apps to transfer the data from
  PostprocessorName _postprocessor;
  /// Variable in the main application to fill with the interpolation of the postprocessors
  AuxVariableName _to_var_name;

  /// Number of points to consider for the interpolation
  unsigned int _num_points;
  /// Exponent for power-law decrease of the interpolation coefficients
  Real _power;
  /// Type of interpolation method
  MooseEnum _interp_type;
  /// Radius to consider for inverse interpolation
  Real _radius;

  /// Whether the target variable is nodal or elemental
  bool _nodal;

private:
  bool usesMooseAppCoordTransform() const override { return true; }
};
