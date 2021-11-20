//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralVariablePostprocessor.h"

#include "libmesh/mesh_tools.h"

/**
 * Computes the average value of a variable on each block
 */
class BlockAverageValue : public ElementIntegralVariablePostprocessor
{
public:
  BlockAverageValue(const InputParameters & parameters);

  static InputParameters validParams();

  /**
   * Given a block ID return the average value for a variable on that block
   *
   * Note that accessor functions on UserObjects like this _must_ be const.
   * That is because the UserObject system returns const references to objects
   * trying to use UserObjects.  This is done for parallel correctness.
   *
   * @return The average value of a variable on that block.
   */
  Real averageValue(SubdomainID block) const;

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
  virtual void threadJoin(const UserObject & y) override;

  /**
   * Called _once_ after execute has been called all all "objects".
   */
  virtual void finalize() override;

protected:
  // This map will hold the partial sums for each block
  std::map<SubdomainID, Real> _integral_values;

  // This map will hold the partial volume sums for each block
  std::map<SubdomainID, Real> _volume_values;

  // This map will hold our averages for each block
  std::map<SubdomainID, Real> _average_values;
};
