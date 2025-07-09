//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"

#include "libmesh/mesh_tools.h"
#include "EulerAngles.h"

/**
 * Computes the average value of a variable on each block
 */
class ComputeBlockOrientationBase : public ElementUserObject
{
public:
  ComputeBlockOrientationBase(const InputParameters & parameters);

  static InputParameters validParams();

  /**
   * Given a block ID return the block orientation of that block
   *
   * Note that accessor functions on UserObjects like this _must_ be const.
   * That is because the UserObject system returns const references to objects
   * trying to use UserObjects.  This is done for parallel correctness.
   *
   * @return The average value of a variable on that block.
   */
  virtual EulerAngles getBlockOrientation(SubdomainID block) const;

  /**
   * This is called before execute so you can reset any internal data.
   */
  virtual void initialize() override;

  /**
   * Called _once_ after execute has been called on all "objects".
   */
  virtual void finalize() override {};

protected:
  // This map will hold our averages for each block
  std::map<SubdomainID, EulerAngles> _block_ea_values;
};
