//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"

// System includes
#include <string>

class InputParameters;

namespace libMesh
{
class Point;
}

template <typename T>
InputParameters validParams();

/**
 * ConstantSolutionOldIC just returns a constant value.
 */
class ConstantSolutionOldIC : public InitialCondition
{
public:
  static InputParameters validParams();

  ConstantSolutionOldIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

  virtual void compute() override;

  virtual void computeNodal(const Point & p) override;

  friend class InitialConditionTempl;

protected:
  const Real _value;
  const bool _set_old_soln_state;
  const int _old_soln_state_number;
};
