//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

/**
 * Sums into _total
 * This is used, for instance, to record the total mass
 * flowing into a borehole.
 * This is a suboptimal setup because it requires a const_cast
 * of a PorousFlowSumQuantity object in order to do the summing
 */
class PorousFlowSumQuantity : public GeneralUserObject
{
public:
  static InputParameters validParams();

  PorousFlowSumQuantity(const InputParameters & parameters);
  virtual ~PorousFlowSumQuantity();

  /// Sets _total = 0
  void zero();

  /**
   * Adds contrib to _total
   * @param contrib the amount to add to _total
   */
  void add(Real contrib);

  /// Does nothing
  virtual void initialize() override;

  /// Does nothing
  virtual void execute() override;

  /// Does MPI gather on _total
  virtual void finalize() override;

  /// Returns _total
  virtual Real getValue() const;

protected:
  /// This holds the sum
  Real _total;
};
