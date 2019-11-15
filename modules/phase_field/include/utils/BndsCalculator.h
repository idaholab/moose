//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

namespace BndsCalculator
{

template <typename T>
Real
computeBndsVariable(const std::vector<T *> & var, unsigned int qp)
{
  Real value = 0.0;

  for (unsigned int i = 0; i < var.size(); ++i)
    value += (*var[i])[qp] * (*var[i])[qp];

  return value;
}

}
