/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef SUBBLOCKINDEXPROVIDER_H
#define SUBBLOCKINDEXPROVIDER_H

#include "GeneralUserObject.h"

class SubblockIndexProvider;

template <>
InputParameters validParams<SubblockIndexProvider>();

/**
 * Abstract base class for user objects that provide an index for a given element that is
 * independent of the block id, so that behavior can be different on subsets of element blocks.
 * This is used to apply independent generalized plane constraints to subsets of element blocks.
 */
class SubblockIndexProvider : public GeneralUserObject
{
public:
  SubblockIndexProvider(const InputParameters & params) : GeneralUserObject(params) {}

  /**
   * The index of subblock this element is on.
   */
  virtual unsigned int getSubblockIndex(const Elem & /* elem */) const = 0;

  /**
   * The max index of subblock.
   */
  virtual unsigned int getMaxSubblockIndex() const = 0;
};

#endif /* SCALARVARIABLEINDEXPROVIDER_H */
