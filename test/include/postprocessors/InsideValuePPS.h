//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

class InsideUserObject;

/**
 * This PPS just retrieves the value from InsideUserObject
 */
class InsideValuePPS : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  InsideValuePPS(const InputParameters & parameters);
  virtual ~InsideValuePPS();

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();

protected:
  const InsideUserObject & _uo;
  Real _value;
};
