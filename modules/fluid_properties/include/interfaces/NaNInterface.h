//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NANINTERFACE_H
#define NANINTERFACE_H

#include "InputParameters.h"

class NaNInterface;
class MooseObject;

template <>
InputParameters validParams<NaNInterface>();

/**
 * Interface class for getting quiet or signaling NaNs
 *
 * For some objects, it is sometimes desirable to have the choice of whether
 * to use a signaling NaN or a quiet NaN. For example, for some simulations,
 * one may want to run in debug mode but not crash on a NaN from a certain
 * object. This class adds a parameter to control this behavior and an interface
 * for getting the corresponding NaN.
 */
class NaNInterface
{
public:
  NaNInterface(const MooseObject * moose_object);

protected:
  /// Use quiet NaNs instead of signaling NaNs?
  const bool _use_quiet_nans;

  /**
   * Returns a quiet or signaling NaN, as specified by the user
   */
  Real getNaN() const;
};

#endif /* NANINTERFACE_H */
