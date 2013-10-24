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

#ifndef INTERNALSIDEUSEROBJECT_H
#define INTERNALSIDEUSEROBJECT_H

#include "UserObject.h"
#include "Coupleable.h"
#include "MaterialPropertyInterface.h"
#include "MooseVariableDependencyInterface.h"
#include "UserObjectInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "ZeroInterface.h"

class InternalSideUserObject;

template<>
InputParameters validParams<InternalSideUserObject>();

/**
 *
 */
class InternalSideUserObject :
  public UserObject,
  public MaterialPropertyInterface,
  public NeighborCoupleable,
  public MooseVariableDependencyInterface,
  public UserObjectInterface,
  public TransientInterface,
  public PostprocessorInterface,
  public ZeroInterface
{
public:
  InternalSideUserObject(const std::string & name, InputParameters parameters);
  virtual ~InternalSideUserObject();

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
  MooseMesh & _mesh;

  const MooseArray<Point> & _q_point;
  QBase * & _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;
  const MooseArray<Point> & _normals;

  const Elem * & _current_elem;
  /// current side of the current element
  unsigned int & _current_side;

  const Elem * & _current_side_elem;
  const Real & _current_side_volume;

  /// The neighboring element
  const Elem * & _neighbor_elem;
  /// The volume (or length) of the current neighbor
  const Real & _neighbor_elem_volume;
};


#endif /* INTERNALSIDEUSEROBJECT_H */
