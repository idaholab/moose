//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERFACEVALUEUSEROBJECT_H
#define INTERFACEVALUEUSEROBJECT_H

#include "InterfaceUserObject.h"

class InterfaceValueUserObject;

template <>
InputParameters validParams<InterfaceValueUserObject>();

/**
 *  A special InterfaceUserObject computing average values across an interface given
 *  the average type (see InterfaceValueTools for details)
 */
class InterfaceValueUserObject : public InterfaceUserObject
{
public:
  InterfaceValueUserObject(const InputParameters & parameters);

protected:
  /// the average type to be computed across the interface
  const MooseEnum _interface_value_type;
  virtual Real computeInterfaceValueType(const Real /*value_master*/, const Real /*value_slave*/);
};

#endif /* INTERFACEVALUEUSEROBJECT_H */
