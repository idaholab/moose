/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef POROUSFLOWSUMQUANTITY_H
#define POROUSFLOWSUMQUANTITY_H

#include "GeneralUserObject.h"

class PorousFlowSumQuantity;

template<>
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
