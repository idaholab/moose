//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "StochasticToolsApp.h"
#include "MooseObject.h"
#include "libmesh/utility.h"
#include "ParallelAcquisitionInterface.h"

/**
 * All ParallelAcquisition functions should inherit from this class
 */
class ParallelAcquisitionFunctionBase : public MooseObject, public ParallelAcquisitionInterface
{
public:
  static InputParameters validParams();
  ParallelAcquisitionFunctionBase(const InputParameters & parameters);

  /**
   * Compute the acquisition function values
   * @param acq The computed acquisition function values
   * @param gp_mean The provided GP mean values
   * @param gp_std The provided GP standard deviation values
   * @param test_inputs All the input values under which the GP has to be tested
   * @param train_inputs All the input values under which the GP has been trained
   * @param generic A generic parameter (can be output values under which the GP has been trained or threshold parameter under the U-Function etc.)
   */
  virtual void computeAcquisition(std::vector<Real> & acq,
                                  const std::vector<Real> & gp_mean,
                                  const std::vector<Real> & gp_std,
                                  const std::vector<std::vector<Real>> & test_inputs,
                                  const std::vector<std::vector<Real>> & train_inputs,
                                  const std::vector<Real> & generic) const = 0;

  /**
   * Return the modified acquisition function values and sorted indices considering local penalization
   * (inspired from Zhan et al. 2017)
   * @param modified_acq The modified acquisition function values
   * @param sorted_indices The sorted indices modified acquisition function values
   * @param acq The originally acquisition function values
   * @param length_scales The length scales to compute the correlation between inputs
   * @param inputs All the input values under which acquisition needs to be computed
   */
  void penalizeAcquisition(std::vector<Real> & modified_acq,
                           std::vector<unsigned int> & sorted_indices,
                           const std::vector<Real> & acq,
                           const std::vector<Real> & length_scales,
                           const std::vector<std::vector<Real>> & inputs);

  /**
   * Compute the correlation between two inputs using the length scales
   * @param corr The computed correlation value
   * @param input1 The first input
   * @param input2 The second input
   * @param length_scales The length scales to compute the correlation between inputs
   */
  void computeCorrelation(Real & corr,
                          const std::vector<Real> & input1,
                          const std::vector<Real> & input2,
                          const std::vector<Real> & length_scales);
};
