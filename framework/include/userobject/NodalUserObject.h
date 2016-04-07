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

  virtual void subdomainSetup() /*final*/;

  bool isUniqueNodeExecute() { return _unique_node_execute; }

protected:
  /// Thread ID of this postprocessor
  THREAD_ID _tid;

  /// Assembly of this thread of this postprocessor
  Assembly & _assembly;

  /// Coordinate system
  const Moose::CoordinateSystemType & _coord_sys;

  /// Quadrature point index
  const unsigned int _qp;

  /// Reference to current node pointer
  const Node * & _current_node;

  // Flag for enable/disabling multiple execute calls on nodes that share block ids
  const bool & _unique_node_execute;
};

#endif
