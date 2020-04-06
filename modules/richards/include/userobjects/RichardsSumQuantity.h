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
 * of a RichardsSumQuantity object in order to do the summing
 */
class RichardsSumQuantity : public GeneralUserObject
{
public:
  static InputParameters validParams();

  RichardsSumQuantity(const InputParameters & parameters);
  virtual ~RichardsSumQuantity();

  /// sets _total = 0
  void zero();

  /**
   * adds contrib to _total
   * @param contrib the amount to add to _total
   */
  void add(Real contrib);

  /// does nothing
  virtual void initialize();

  /// does nothing
  virtual void execute();

  /// does MPI gather on _total
  virtual void finalize();

  /// returns _total
  virtual Real getValue() const;

protected:
  /// this holds the sum
  Real _total;
};
