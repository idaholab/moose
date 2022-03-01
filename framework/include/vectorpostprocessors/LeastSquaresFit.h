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
 *  LeastSquaresFit is a VectorPostprocessor that performs a least squares
 *  fit on data calculated in another VectorPostprocessor.
 */

class LeastSquaresFit : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters The input parameters
   */
  LeastSquaresFit(const InputParameters & parameters);

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

  /// The type of output
  const MooseEnum _output_type;

  /// The number of samples to be taken
  unsigned int _num_samples;

  ///@{ Values used to scale and or shift x and y data
  const Real _x_scale;
  const Real _x_shift;
  const Real _y_scale;
  const Real _y_shift;
  ///@}

  /// Did the user specify the min and max x values for sampling?
  bool _have_sample_x_min;
  bool _have_sample_x_max;

  ///@{ The min and max x values for sampling
  Real _sample_x_min;
  Real _sample_x_max;
  ///@}

  ///@{ The variables used to write out samples of the least squares fit
  VectorPostprocessorValue * _sample_x;
  VectorPostprocessorValue * _sample_y;
  ///@}

  /// The variable used to write out the coefficients of the fit
  VectorPostprocessorValue * _coeffs;
};
