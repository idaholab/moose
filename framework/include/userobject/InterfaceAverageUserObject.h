//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERFACEAVERAGEUSEROBJECT_H
#define INTERFACEAVERAGEUSEROBJECT_H

#include "InterfaceUserObject.h"

class InterfaceAverageUserObject;

template <>
InputParameters validParams<InterfaceAverageUserObject>();

/**
 *  A special InterfaceUserObject computing average values across an interface given
 *  the average type (see InterfaceAverageTools for details)
 */
class InterfaceAverageUserObject : public InterfaceUserObject
{
public:
  InterfaceAverageUserObject(const InputParameters & parameters);

protected:
  /// the average type to be computed across the interface
  const MooseEnum _average_type;
  virtual Real ComputeAverageType(const Real /*value_master*/, const Real /*value_slave*/);
};

#endif /* INTERFACEAVERAGEUSEROBJECT_H */
