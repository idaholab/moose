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

//MOOSE includes
#include "MooseObject.h"
#include "SetupInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "TransientInterface.h"
#include "GeometricSearchInterface.h"
#include "Restartable.h"
#include "ZeroInterface.h"

#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseMesh.h"

//libMesh includes
#include "libmesh/libmesh_common.h"
#include "libmesh/elem.h"
#include "libmesh/point.h"

//Forward Declarations
class Assembly;
class Constraint;

template<>
InputParameters validParams<Constraint>();

/**
 * Base class for all Constraint types
 */
class Constraint :
  public MooseObject,
  public SetupInterface,
  public FunctionInterface,
  public UserObjectInterface,
  public TransientInterface,
  protected GeometricSearchInterface,
  public Restartable,
  public ZeroInterface
{
public:
  Constraint(const std::string & name, InputParameters parameters);
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
