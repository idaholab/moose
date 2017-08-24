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

#ifndef CONSTRAINT_H
#define CONSTRAINT_H

// MOOSE includes
#include "MooseObject.h"
#include "SetupInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "TransientInterface.h"
#include "GeometricSearchInterface.h"
#include "Restartable.h"
#include "ZeroInterface.h"
#include "MeshChangedInterface.h"

// Forward Declarations
class Assembly;
class Constraint;
class MooseVariable;
class SubProblem;
class MooseMesh;

template <>
InputParameters validParams<Constraint>();

/**
 * Base class for all Constraint types
 */
class Constraint : public MooseObject,
                   public SetupInterface,
                   public FunctionInterface,
                   public UserObjectInterface,
                   public TransientInterface,
                   protected GeometricSearchInterface,
                   public Restartable,
                   public ZeroInterface,
                   public MeshChangedInterface
{
public:
  Constraint(const InputParameters & parameters);
  virtual ~Constraint();

  /**
   * Subproblem this constraint is part of
   * @return The reference to the subproblem
   */
  SubProblem & subProblem() { return _subproblem; }

  /**
   * The variable number that this object operates on.
   */
  MooseVariable & variable() { return _var; }

  virtual bool addCouplingEntriesToJacobian() { return true; }
  virtual void subdomainSetup() override final
  {
    mooseError("subdomain setup for constraints is not implemented");
  }

  virtual bool contactConverged() { return false; }

  virtual void updateLagMul(bool) { return; }

  virtual bool haveAugLM() { return false; }

protected:
  SubProblem & _subproblem;
  SystemBase & _sys;

  THREAD_ID _tid;

  Assembly & _assembly;
  MooseVariable & _var;
  MooseMesh & _mesh;

  unsigned int _i, _j;
  unsigned int _qp;
};

#endif
