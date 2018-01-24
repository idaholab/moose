//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NEIGHBORMOOSEVARIABLEINTERFACE_H
#define NEIGHBORMOOSEVARIABLEINTERFACE_H

#include "MooseVariableInterface.h"
#include "MooseVariableBase.h"

/**
 * Enhances MooseVariableInterface interface provide values from neighbor elements
 *
 */
class NeighborMooseVariableInterface : public MooseVariableInterface
{
public:
  /**
   * Constructing the object
   * @param parameters Parameters that come from constructing the object
   * @param nodal true if the variable is nodal
   */
  NeighborMooseVariableInterface(const MooseObject * moose_object, bool nodal);

  virtual ~NeighborMooseVariableInterface();

protected:
  /**
   * The value of the variable this object is operating on evaluated on the "neighbor" element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableValue & neighborValue();

  /**
   * The old value of the variable this object is operating on evaluated on the "neighbor" element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableValue & neighborValueOld();

  /**
   * The older value of the variable this object is operating on evaluated on the "neighbor"
   * element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableValue & neighborValueOlder();

  /**
   * The gradient of the variable this object is operating on evaluated on the "neighbor" element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableGradient & neighborGradient();

  /**
   * The old gradient of the variable this object is operating on evaluated on the "neighbor"
   * element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableGradient & neighborGradientOld();

  /**
   * The older gradient of the variable this object is operating on evaluated on the "neighbor"
   * element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableGradient & neighborGradientOlder();

  /**
   * The second derivative of the variable this object is operating on evaluated on the "neighbor"
   * element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableSecond & neighborSecond();

  /**
   * The old second derivative of the variable this object is operating on evaluated on the
   * "neighbor" element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableSecond & neighborSecondOld();

  /**
   * The older second derivative of the variable this object is operating on evaluated on the
   * "neighbor" element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableSecond & neighborSecondOlder();

  /**
   * The second derivative of the neighbor's test function.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariableTestSecond & neighborSecondTest();

  /**
   * The second derivative of the neighbor's shape function.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const VariablePhiSecond & neighborSecondPhi();
};

#endif /* NEIGHBORMOOSEVARIABLEINTERFACE_H */
