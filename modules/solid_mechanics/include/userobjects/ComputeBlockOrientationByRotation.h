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
   * This is called before execute so you can reset any internal data.
   */
  virtual void initialize() override;

  /**
   * Called on every "object" (like every element or node).
   * In this case, it is called at every quadrature point on every element.
   */
  virtual void execute() override;

  /**
   * Called when using threading.  You need to combine the data from "y"
   * into _this_ object.
   */
  virtual void threadJoin(const UserObject & /*y*/) override {};

  /**
   * Called _once_ after execute has been called all all "objects".
   */
  virtual void finalize() override;

protected:
  /**
   * Compute Quaternion for each subdomain (block)
   */
  EulerAngles computeSubdomainEulerAngles(const SubdomainID & sid);

  // updated quaternion
  const MaterialProperty<RankTwoTensor> & _updated_rotation;

  /// number of bins for each quaternion component
  unsigned int _bins;

  /// L_norm value for averaging
  Real _L_norm;

  // Array of vectors to store quaternions of each grain
  std::unordered_map<SubdomainID, std::vector<std::tuple<Real, Real, Real, Real>>> _quat;
};
