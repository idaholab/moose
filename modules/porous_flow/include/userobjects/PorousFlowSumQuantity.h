//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWSUMQUANTITY_H
#define POROUSFLOWSUMQUANTITY_H

#include "GeneralUserObject.h"

class PorousFlowSumQuantity;

template <>
InputParameters validParams<PorousFlowSumQuantity>();

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
  PorousFlowSumQuantity(const InputParameters & parameters);
  virtual ~PorousFlowSumQuantity();

  /// sets _total = 0
  void zero();

  /**
   * adds contrib to _total
   * @param contrib the amount to add to _total
   */
  void add(Real contrib);

  /// does nothing
  virtual void initialize() override;

  /// does nothing
  virtual void execute() override;

  /// does MPI gather on _total
  virtual void finalize() override;

  /// returns _total
  virtual Real getValue() const;

protected:
  /// this holds the sum
  Real _total;
};

#endif /* POROUSFLOWSUMQUANTITY_H */
