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

#ifndef NEIGHBORMOOSEVARIABLEINTERFACE_H
#define NEIGHBORMOOSEVARIABLEINTERFACE_H

#include "MooseVariableInterface.h"
#include "MooseVariable.h"
#include "InputParameters.h"

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
