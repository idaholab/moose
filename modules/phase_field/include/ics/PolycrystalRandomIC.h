//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RandomICBase.h"

/**
 * Random initial condition for a polycrystalline material
 */
class PolycrystalRandomIC : public RandomICBase
{
public:
  static InputParameters validParams();

  PolycrystalRandomIC(const InputParameters & parameters);

  virtual Real value(const Point & p);

private:
  const unsigned int _op_num;
  const unsigned int _op_index;
  const unsigned int _random_type;
};
