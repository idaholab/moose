//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalUserObject.h"
#include "MortarConsumerInterface.h"
#include "MortarExecutorInterface.h"
#include "TwoMaterialPropertyInterface.h"

/**
 * Base class for creating new nodally-based mortar user objects
 */
class MortarNodalUserObject : public NodalUserObject,
                              public MortarExecutorInterface,
                              public MortarConsumerInterface,
                              public TwoMaterialPropertyInterface
{
public:
  static InputParameters validParams();

  MortarNodalUserObject(const InputParameters & parameters);

  virtual void execute() override;
  virtual void initialSetup() override;

protected:
  virtual void executeMortarSegment() = 0;

  /// Whether we're computing on the displaced mesh
  const bool _displaced;

  /// The base finite element problem
  FEProblemBase & _fe_problem;
};
