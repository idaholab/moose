//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeBlockOrientationBase.h"

/**
 * Computes the average value of a variable on each block
 */
class ComputeBlockOrientationByRotation : public ComputeBlockOrientationBase
{
public:
  ComputeBlockOrientationByRotation(const InputParameters & parameters);

  static InputParameters validParams();

  /**
   * Clear internal Euler angle and misorientationdata.
   */
  virtual void initialize() override;

  /**
   * Compute the average of the rotation matrix in this element
   */
  virtual void execute() override;

  virtual void threadJoin(const UserObject & /*y*/) override {};

  /**
   * Gather all Euler angles from this block
   */
  virtual void finalize() override;

protected:
  /**
   * Compute Quaternion for each subdomain (block), following
   * Markley, F. Landis, Yang Cheng, John Lucas Crassidis, and Yaakov Oshman.
   * "Averaging quaternions." Journal of Guidance, Control, and Dynamics 30,
   * no. 4 (2007): 1193-1197.
   */
  EulerAngles computeSubdomainEulerAngles(const SubdomainID & sid);

  // updated quaternion
  const MaterialProperty<RankTwoTensor> & _updated_rotation;

  /// number of bins for each quaternion component
  unsigned int _bins;

  /// parameter used to compute the weighting function for the average quaternion calculation
  Real _L_norm;

  // Array of vectors to store quaternions of each grain
  std::unordered_map<SubdomainID, std::vector<std::tuple<Real, Real, Real, Real>>> _quat;
};
