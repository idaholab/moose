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

#ifndef ELEMENTUSEROBJECT_H
#define ELEMENTUSEROBJECT_H

#include "MooseVariable.h"
#include "UserObject.h"
#include "UserObjectInterface.h"
#include "Coupleable.h"
#include "ScalarCoupleable.h"
#include "MooseVariableDependencyInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "BlockRestrictable.h"
#include "MaterialPropertyInterface.h"
#include "RandomInterface.h"
#include "ZeroInterface.h"
// libMesh
#include "libmesh/elem.h"
#include "MooseTypes.h"

//Forward Declarations
class ElementUserObject;

template<>
InputParameters validParams<ElementUserObject>();

class ElementUserObject :
  public UserObject,
  public BlockRestrictable,
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
  ElementUserObject(const InputParameters & parameters);

  /**
   * This function will get called on each geometric object this postprocessor acts on
   * (ie Elements, Sides or Nodes).  This will most likely get called multiple times
   * before getValue() is called.
   *
   * Someone somewhere has to override this.
   */
  virtual void execute() = 0;

  /**
   * This function will be called with the shape functions for jvar initialized. It
   * can be used to compute Jacobian contributions of the UserObject.
   */
  virtual void executeJacobian(unsigned int /*jvar*/) {}

  /**
   * Must override.
   *
   * @param uo The UserObject to be joined into _this_ object.  Take the data from the uo object and "add" it into the data for this object.
   */
  virtual void threadJoin(const UserObject & uo) = 0;

  bool requestedJacobian(unsigned int var) const;

protected:
  MooseMesh & _mesh;

  /// The current element pointer (available during execute())
  const Elem * & _current_elem;

  /// The current element volume (available during execute())
  const Real & _current_elem_volume;

  const MooseArray< Point > & _q_point;
  QBase * & _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;

  /// flag store to indicate if a jacobian w.r.t. a non-linear variable with a given number is requested
  std::vector<bool> _requested_jacobian_flag;
};

#endif
