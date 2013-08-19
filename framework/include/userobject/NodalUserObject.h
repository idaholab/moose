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

#include "UserObject.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "UserObjectInterface.h"
#include "MooseVariable.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "BlockRestrictable.h"

class MooseVariable;

//Forward Declarations
class NodalUserObject;

template<>
InputParameters validParams<NodalUserObject>();

class NodalUserObject :
  public UserObject,
  public BlockRestrictable,
  public UserObjectInterface,
  public Coupleable,
  public ScalarCoupleable,
  public MooseVariableDependencyInterface,
  public TransientInterface,
  protected PostprocessorInterface
{
public:
  NodalUserObject(const std::string & name, InputParameters parameters);

  const std::vector<BoundaryName> & boundaries() { return _boundaries; }

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
  std::vector<BoundaryName> _boundaries;

  const unsigned int _qp;
  const Node * & _current_node;

  // Single Instance Variables
  Real & _real_zero;
  MooseArray<Real> & _zero;
};

#endif
