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
#include "Assembly.h"
#include "ADFunctorInterface.h"
#include "MooseLinearVariableFV.h"
#include "MooseVariableInterface.h"
#include "MooseVariableDependencyInterface.h"

class SubProblem;

class LinearFVKernel : public LinearSystemContributionObject,
                       public BlockRestrictable,
                       public NonADFunctorInterface,
                       public MooseVariableInterface<Real>,
                       public MooseVariableDependencyInterface
{
public:
  static InputParameters validParams();
  static void setRMParams(const InputParameters & obj_params,
                          InputParameters & rm_params,
                          unsigned short ghost_layers);
  LinearFVKernel(const InputParameters & params);

  virtual const MooseLinearVariableFV<Real> & variable() const override { return *_var; }

protected:
  /// Pointer to the linear finite volume variable
  MooseLinearVariableFV<Real> * _var;
};
