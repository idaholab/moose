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

#include "GeneralPostprocessor.h"

class InterfaceUOPPS;
class InterfaceUO;

template <>
InputParameters validParams<InterfaceUOPPS>();

/**
 * This PPS just retrieves the value from InterfaceUO
 */
class InterfaceUOPPS : public GeneralPostprocessor
{
public:
  InterfaceUOPPS(const InputParameters & parameters);
  virtual ~InterfaceUOPPS();

  virtual void initialize();
  virtual void execute();
  virtual Real getMeanMatProp();
  virtual Real getMeanVarJump();
  virtual Real getValue();

protected:
  const InterfaceUO & _uo;
  Real _mean_mat_prop;
  Real _mean_var_jump;
};

#endif // INTERFACEUOPPS_H
