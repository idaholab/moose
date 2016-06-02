/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef INDICATOR_H
#define INDICATOR_H

#include "MooseObject.h"
#include "BlockRestrictable.h"
#include "SetupInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "Restartable.h"
#include "OutputInterface.h"

// Forward Declarations
class Indicator;
class MooseMesh;
class SubProblem;
class Assembly;

template<>
InputParameters validParams<Indicator>();

class Indicator :
  public MooseObject,
  public BlockRestrictable,
  public SetupInterface,
  public FunctionInterface,
  public UserObjectInterface,
  public Restartable,
  public OutputInterface
{
public:
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
  virtual void finalize() {};

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
#endif /* INDICATOR_H */
