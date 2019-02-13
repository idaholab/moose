//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERFACEUO_H
#define INTERFACEUO_H

#include "InterfaceAverageUserObject.h"

class InterfaceUO;

template <>
InputParameters validParams<InterfaceAverageUserObject>();

/**
 *
 */
class InterfaceUO : public InterfaceAverageUserObject
{
public:
  InterfaceUO(const InputParameters & parameters);
  virtual ~InterfaceUO();

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & /*uo*/) override;

  Real getValue() const;

protected:
  const VariableValue & _u;
  const VariableValue & _u_neighbor;

  Real _value;
  Real _total_volume;
};

#endif // INTERFACEUO_H
