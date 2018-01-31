//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

  virtual void residualEnd() {}

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
