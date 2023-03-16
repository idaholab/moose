//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceUserObject.h"

/**
 *  A special InterfaceUserObject computing average values across an interface given
 *  the average type (see InterfaceValueTools for details)
 */
class InterfaceValueUserObject : public InterfaceUserObject
{
public:
  static InputParameters validParams();
  InterfaceValueUserObject(const InputParameters & parameters);

protected:
  /**
   * method computing an interface value give two Real quantities
   **/
  virtual Real computeInterfaceValueType(const Real /*value_primary*/,
                                         const Real /*value_secondary*/);

  /// the average type to be computed across the interface
  const MooseEnum _interface_value_type;
};
