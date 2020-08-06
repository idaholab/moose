//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ACBulk.h"

// Forward Declarations

/**
 * This is the base class for kernels that calculate the residual for grain growth.
 * It calculates the residual of the ith order parameter, and the values of
 * all other order parameters are coupled variables and are stored in vals.
 */
class ACGrGrBase : public ACBulk<Real>
{
public:
  static InputParameters validParams();

  ACGrGrBase(const InputParameters & parameters);

protected:
  const unsigned int _op_num;

  const std::vector<const VariableValue *> _vals;
  const std::vector<unsigned int> _vals_var;

  const MaterialProperty<Real> & _mu;
};
