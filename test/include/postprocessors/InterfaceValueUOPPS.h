//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERFACEVALUEUOPPS_H
#define INTERFACEVALUEUOPPS_H

#include "GeneralPostprocessor.h"

class InterfaceValueUOPPS;
class InterfaceValueAverageUO;

template <>
InputParameters validParams<InterfaceValueUOPPS>();

/**
 * This PPS just retrieves the value from InterfaceUO_MP
 */
class InterfaceValueUOPPS : public GeneralPostprocessor
{
public:
  InterfaceValueUOPPS(const InputParameters & parameters);
  virtual ~InterfaceValueUOPPS();

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;
  // threadJoin is already performed in the userobject, so do nothing
  virtual void threadJoin(const UserObject & /*uo*/) override{};

protected:
  const InterfaceValueAverageUO & _uo;
  Real _value_pp;
};

#endif // INTERFACEVALUEUOPPS_H
