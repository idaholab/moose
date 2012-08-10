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
#include "SetupInterface.h"

#include "Coupleable.h"
#include "MooseVariableInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "MaterialPropertyInterface.h"

#include "Assembly.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseTypes.h"


class MooseMesh;
class Problem;
class SubProblem;

//Forward Declarations
class Indicator;

template<>
InputParameters validParams<Indicator>();

class Indicator :
  public MooseObject,
  public SetupInterface,
  public Coupleable,
  public ScalarCoupleable,
  public MooseVariableInterface,
  public FunctionInterface,
  public UserObjectInterface,
  public MaterialPropertyInterface
{
public:
  Indicator(const std::string & name, InputParameters parameters);
  virtual ~Indicator(){};

  virtual void computeIndicator() = 0;

  /**
   * The variable number that this Indicator operates on.
   */
//  MooseVariable & variable() { return _var; }

  SubProblem & subProblem() { return _subproblem; }

  const bool isActive() { return true; }

  virtual void IndicatorSetup();

protected:
  SubProblem & _subproblem;
  FEProblem & _fe_problem;
  SystemBase & _sys;

  THREAD_ID _tid;

  Assembly & _assembly;

  MooseVariable & _field_var;
//  const MooseVariable & _var;

  MooseMesh & _mesh;
  unsigned int _dim;
};
#endif /* INDICATOR_H */
