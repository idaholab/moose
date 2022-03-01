//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "BlockRestrictable.h"
#include "SetupInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "MooseVariableDependencyInterface.h"
#include "Restartable.h"
#include "OutputInterface.h"
#include "MaterialPropertyInterface.h"

class MooseMesh;
class SubProblem;
class Assembly;

class Indicator : public MooseObject,
                  public BlockRestrictable,
                  public SetupInterface,
                  public FunctionInterface,
                  public UserObjectInterface,
                  public MooseVariableDependencyInterface,
                  public Restartable,
                  public OutputInterface,
                  public MaterialPropertyInterface
{
public:
  static InputParameters validParams();

  Indicator(const InputParameters & parameters);

  virtual ~Indicator(){};

  /**
   * Pure virtual that must be overridden.
   *
   * This is generally overridden by an intermediate base class.
   * Usually you will want to inherit from InternalSideIndicator or ElementIndicator.
   * They contain other virtual functions you will probably want to override instead.
   */
  virtual void computeIndicator() = 0;

  /**
   * Can be overridden to do a final postprocessing of the indicator field.
   * This will allow you to sum up error from multiple places and then do something like take the
   * square root of it in this function.
   */
  virtual void finalize(){};

  SubProblem & subProblem() { return _subproblem; }

  // TODO: Fixme
  bool isActive() const { return true; }

protected:
  SubProblem & _subproblem;
  FEProblemBase & _fe_problem;
  SystemBase & _sys;
  NumericVector<Number> & _solution;

  THREAD_ID _tid;

  Assembly & _assembly;

  MooseMesh & _mesh;
};
