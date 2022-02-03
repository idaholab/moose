//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseVariableInterface.h"
#include "MooseVariableBase.h"

/**
 * Enhances MooseVariableInterface interface provide values from neighbor elements
 *
 */
template <typename T>
class NeighborMooseVariableInterface : public MooseVariableInterface<T>
{
public:
  /**
   * Constructing the object
   * @param parameters Parameters that come from constructing the object
   * @param nodal true if the variable is nodal
   */
  NeighborMooseVariableInterface(
      const MooseObject * moose_object,
      bool nodal,
      Moose::VarKindType expected_var_type = Moose::VarKindType::VAR_ANY,
      Moose::VarFieldType expected_var_field_type = Moose::VarFieldType::VAR_FIELD_STANDARD);

  virtual ~NeighborMooseVariableInterface();

protected:
  /**
   * The value of the variable this object is operating on evaluated on the "neighbor" element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableValue & neighborValue();

  /**
   * The old value of the variable this object is operating on evaluated on the "neighbor" element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableValue & neighborValueOld();

  /**
   * The older value of the variable this object is operating on evaluated on the "neighbor"
   * element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableValue & neighborValueOlder();

  /**
   * The gradient of the variable this object is operating on evaluated on the "neighbor" element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableGradient & neighborGradient();

  /**
   * The old gradient of the variable this object is operating on evaluated on the "neighbor"
   * element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableGradient & neighborGradientOld();

  /**
   * The older gradient of the variable this object is operating on evaluated on the "neighbor"
   * element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableGradient & neighborGradientOlder();

  /**
   * The second derivative of the variable this object is operating on evaluated on the "neighbor"
   * element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableSecond & neighborSecond();

  /**
   * The old second derivative of the variable this object is operating on evaluated on the
   * "neighbor" element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableSecond & neighborSecondOld();

  /**
   * The older second derivative of the variable this object is operating on evaluated on the
   * "neighbor" element.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableSecond & neighborSecondOlder();

  /**
   * The second derivative of the neighbor's test function.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariableTestSecond & neighborSecondTest();

  /**
   * The second derivative of the neighbor's shape function.
   *
   * @return The reference to be stored off and used later.
   */
  virtual const typename OutputTools<T>::VariablePhiSecond & neighborSecondPhi();
};
