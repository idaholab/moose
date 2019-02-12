//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERFACEUOPPS_H
#define INTERFACEUOPPS_H

#include "InterfacePostprocessor.h"

class InterfaceUOPPS;
class InterfaceUO;

template <>
InputParameters validParams<InterfaceUOPPS>();

/**
 * This PPS just retrieves the value from InterfaceUO_MP
 */
class InterfaceUOPPS : public InterfacePostprocessor
{
public:
  InterfaceUOPPS(const InputParameters & parameters);
  virtual ~InterfaceUOPPS();

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual Real getValue() override;
  // threadJoin is already performed in the userobject, so do nothing
  virtual void threadJoin(const UserObject & /*uo*/) override{};

protected:
  const InterfaceUO & _uo;
  Real _value_pp;
};

#endif // INTERFACEUOPPS_H
