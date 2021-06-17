//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADNodalKernel.h"

/**
 * Class used to enforce a upper bound on a coupled variable
 */
class ADUpperBoundNodalKernel : public ADNodalKernel
{
public:
  static InputParameters validParams();

  ADUpperBoundNodalKernel(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

private:
  /// The number of the coupled variable
  const unsigned int _v_var;

  /// The value of the coupled variable
  const ADVariableValue & _v;

  /// Boundaries on which we should not execute this object
  std::set<BoundaryID> _bnd_ids;

  /// The upper bound on the coupled variable
  const Real _upper_bound;
};
