//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVKernel.h"

#include "MooseVariableInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"

class FVElementalKernel : public FVKernel, public MooseVariableInterface<Real>, public CoupleableMooseVariableDependencyIntermediateInterface
{
public:
  static InputParameters validParams();
  FVElementalKernel(const InputParameters & parameters);

  virtual void computeResidual() = 0;
  virtual void computeJacobian() = 0;
protected:
  MooseVariableFV<Real> & _var;
  const unsigned int _qp = 0;
};

template <ComputeStage compute_stage>
class FVTimeKernel : public FVElementalKernel
{
public:
  static InputParameters validParams();
  FVTimeKernel(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;

protected:
  virtual ADReal computeQpResidual();

  const ADVariableValue & _u_dot;
};

