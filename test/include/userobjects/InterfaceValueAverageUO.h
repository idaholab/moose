//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERFACEVALUEAVERAGEUO_H
#define INTERFACEVALUEAVERAGEUO_H

#include "InterfaceValueUserObject.h"

class InterfaceValueAverageUO;

template <>
InputParameters validParams<InterfaceValueUserObject>();

/**
 * InterfaceValueUserObject is a super class used to compute an average value
 * across an interface
 */
class InterfaceValueAverageUO : public InterfaceValueUserObject
{
public:
  InterfaceValueAverageUO(const InputParameters & parameters);
  virtual ~InterfaceValueAverageUO();

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

#endif // INTERFACEVALUEAVERAGEUO_H
