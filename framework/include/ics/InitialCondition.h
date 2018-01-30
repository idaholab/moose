//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INITIALCONDITION_H
#define INITIALCONDITION_H

#include "MooseObject.h"
#include "Coupleable.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "Restartable.h"
#include "BlockRestrictable.h"
#include "DependencyResolverInterface.h"
#include "BoundaryRestrictable.h"
#include "MooseTypes.h"

// libMesh
#include "libmesh/point.h"
#include "libmesh/vector_value.h"
#include "libmesh/elem.h"

// forward declarations
class InitialCondition;
class FEProblemBase;
class SystemBase;
class Assembly;
class MooseVariable;

template <>
InputParameters validParams<InitialCondition>();

/**
 * InitialConditions are objects that set the initial value of variables.
 */
class InitialCondition : public MooseObject,
                         public BlockRestrictable,
                         public Coupleable,
                         public FunctionInterface,
                         public UserObjectInterface,
                         public BoundaryRestrictable,
                         public DependencyResolverInterface,
                         public Restartable
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  InitialCondition(const InputParameters & parameters);

  virtual ~InitialCondition();

  MooseVariable & variable() { return _var; }

  const std::set<std::string> & getDependObjects() const { return _depend_uo; }

  virtual void compute();

  /**
   * The value of the variable at a point.
   *
   * This must be overridden by derived classes.
   */
  virtual Real value(const Point & p) = 0;

  /**
   * The gradient of the variable at a point.
   *
   * This is optional.  Note that if you are using C1 continuous elements you will
   * want to use an initial condition that defines this!
   */
  virtual RealGradient gradient(const Point & /*p*/) { return RealGradient(); };

  /**
   * Gets called at the beginning of the simulation before this object is asked to do its job.
   * Note: This method is normally inherited from SetupInterface.  However in this case it makes
   * no sense to inherit the other virtuals available in that class so we are adding this virtual
   * directly to this class with out the extra inheritance.
   */
  virtual void initialSetup() {}

  virtual const std::set<std::string> & getRequestedItems() override;

  virtual const std::set<std::string> & getSuppliedItems() override;

  template <typename T>
  const T & getUserObject(const std::string & name);
  template <typename T>
  const T & getUserObjectByName(const UserObjectName & name);

  const UserObject & getUserObjectBase(const std::string & name);

protected:
  FEProblemBase & _fe_problem;
  SystemBase & _sys;
  THREAD_ID _tid;

  Assembly & _assembly;

  /// Time
  Real & _t;

  /// The coordinate system type for this problem, references the value in Assembly
  const Moose::CoordinateSystemType & _coord_sys;

  /// The variable that this initial condition is acting upon.
  MooseVariable & _var;

  /**
   * The current element we are on will retrieving values at specific points in the domain. Note
   * that this _IS_
   * valid even for nodes shared among several elements.
   */
  const Elem *& _current_elem;

  /**
   * The current node if the point we are evaluating at also happens to be a node.
   * Otherwise the pointer will be NULL.
   */
  const Node * _current_node;

  /// The current quadrature point, contains the "nth" node number when visiting nodes.
  unsigned int _qp;

  std::set<std::string> _depend_vars;
  std::set<std::string> _supplied_vars;

  /// Depend UserObjects
  std::set<std::string> _depend_uo;

  /// If set, UOs retrieved by this IC will not be executed before this IC
  const bool _ignore_uo_dependency;
};

template <typename T>
const T &
InitialCondition::getUserObject(const std::string & name)
{
  if (!_ignore_uo_dependency)
    _depend_uo.insert(_pars.get<UserObjectName>(name));
  return UserObjectInterface::getUserObject<T>(name);
}

template <typename T>
const T &
InitialCondition::getUserObjectByName(const UserObjectName & name)
{
  if (!_ignore_uo_dependency)
    _depend_uo.insert(name);
  return UserObjectInterface::getUserObjectByName<T>(name);
}

#endif // INITIALCONDITION_H
