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

#ifndef NODALUSEROBJECT_H
#define NODALUSEROBJECT_H

// MOOSE includes
#include "UserObject.h"
#include "BlockRestrictable.h"
#include "BoundaryRestrictable.h"
#include "MaterialPropertyInterface.h"
#include "UserObjectInterface.h"
#include "Coupleable.h"
#include "ScalarCoupleable.h"
#include "MooseVariableDependencyInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "RandomInterface.h"
#include "ZeroInterface.h"

// Forward Declarations
class NodalUserObject;

template<>
InputParameters validParams<NodalUserObject>();

/**
 * A user object that runs over all the nodes and does an aggregation
 * step to compute a single value.
 */
class NodalUserObject :
  public UserObject,
  public BlockRestrictable,
  public BoundaryRestrictable,
  public MaterialPropertyInterface,
  public UserObjectInterface,
  public Coupleable,
  public ScalarCoupleable,
  public MooseVariableDependencyInterface,
  public TransientInterface,
  protected PostprocessorInterface,
  public RandomInterface,
  public ZeroInterface
{
public:
  NodalUserObject(const InputParameters & parameters);

  /**
   * This function will get called on each geometric object this postprocessor acts on
   * (ie Elements, Sides or Nodes).  This will most likely get called multiple times
   * before getValue() is called.
   *
   * Someone somewhere has to override this.
   */
  virtual void execute() = 0;

  /**
   * Must override.
   *
   * @param uo The UserObject to be joined into _this_ object.  Take the data from the uo object and "add" it into the data for this object.
   */
  virtual void threadJoin(const UserObject & uo) = 0;

protected:
  /// The mesh that is being iterated over
  MooseMesh & _mesh;

  /// Quadrature point index
  const unsigned int _qp;

  /// Reference to current node pointer
  const Node * & _current_node;
};

#endif
