//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

/**
 *  LeastSquaresFitHistory is a VectorPostprocessor that performs a least squares
 *  fit on data calculated in another VectorPostprocessor and stores the full
 *  history of the coefficients.
 */

class LeastSquaresFitHistory : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters The input parameters
   */
  LeastSquaresFitHistory(const InputParameters & parameters);

  /**
   * Initialize, clears old results
   */
  virtual void initialize() override;

  /**
   * Perform the least squares fit
   */
  virtual void execute() override;

protected:
  /// The name of the VectorPostprocessor on which to perform the fit
  VectorPostprocessorName _vpp_name;

  /// The order of the polynomial fit to be performed
  const unsigned int _order;

  /// Whether to truncate the polynomial order if an insufficient number of points is provided
  const bool _truncate_order;

  /// The name of the variables storing the x, y data
  const std::string _x_name;
  const std::string _y_name;

  ///@{ The variables with the x, y data to be fit
  const VectorPostprocessorValue & _x_values;
  const VectorPostprocessorValue & _y_values;
  ///@}

  ///@{ Values used to scale and or shift x and y data
  const Real _x_scale;
  const Real _x_shift;
  const Real _y_scale;
  const Real _y_shift;
  ///@}

  /// Vector of times
  VectorPostprocessorValue * _times;

  /// Vector of vectors with the individual coefficients.
  /// A separate vector is used for the time history of each
  /// coefficient.
  std::vector<VectorPostprocessorValue *> _coeffs;
};
