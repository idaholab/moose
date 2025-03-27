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

#include "libmesh/mesh_tools.h"
#include "EulerAngles.h"

/**
 * Computes the average value of a variable on each block
 */
class ComputeBlockOrientationByMisorientation : public ComputeBlockOrientationBase
{
public:
  ComputeBlockOrientationByMisorientation(const InputParameters & parameters);

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
  virtual void threadJoin(const UserObject & /*y*/) override{};

  /**
   * Called _once_ after execute has been called on all "objects".
   */
  virtual void finalize() override;

  EulerAngles computeSubdomainEulerAngles(const SubdomainID & sid);

protected:
  // updated quaternion
  const MaterialProperty<RankTwoTensor> & _updated_rotation;

  // misorientation angle values
  const MaterialProperty<Real> & _misorient;

  // Array of vectors to store block ID, maximum misorientation angle and corresponding EulerAngle
  std::unordered_map<SubdomainID, std::vector<std::tuple<Real, Real, Real, Real>>>
      _grain_misorientation;
};
