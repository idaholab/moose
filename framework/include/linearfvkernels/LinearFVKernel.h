//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearSystemContributionObject.h"
#include "BlockRestrictable.h"
#include "NonADFunctorInterface.h"
#include "MooseLinearVariableFV.h"
#include "MooseVariableInterface.h"
#include "MooseVariableDependencyInterface.h"
#include "FVRelationshipManagerInterface.h"

/**
 * Base class for finite volume kernels that contribute to a linear
 * systems.
 */
class LinearFVKernel : public LinearSystemContributionObject,
                       public BlockRestrictable,
                       public NonADFunctorInterface,
                       public FVRelationshipManagerInterface,
                       public MooseVariableInterface<Real>,
                       public MooseVariableDependencyInterface
{
public:
  static InputParameters validParams();

  LinearFVKernel(const InputParameters & params);

  virtual const MooseLinearVariableFV<Real> & variable() const override { return _var; }

protected:
  /// Reference to the linear finite volume variable
  MooseLinearVariableFV<Real> & _var;

  /// Cache for the variable number
  const unsigned int _var_num;

  /// Cache for the system number
  const unsigned int _sys_num;
};
