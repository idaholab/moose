//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "Coupleable.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "Restartable.h"
#include "BlockRestrictable.h"
#include "DependencyResolverInterface.h"
#include "BoundaryRestrictable.h"
#include "MooseTypes.h"
#include "MemberTemplateMacros.h"

// forward declarations
class InitialConditionBase;
class SystemBase;
class MooseVariableFEBase;
namespace libMesh
{
class Point;
}

template <>
InputParameters validParams<InitialConditionBase>();

/**
 * InitialConditionBase serves as the abstract base class for InitialConditions and
 * VectorInitialConditions. Implements methods for getting user objects and dependent objects. The
 * template class that inherits from this class implements the meat of the initial condition
 * hierarchy: the `compute` method
 */
class InitialConditionBase : public MooseObject,
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
  InitialConditionBase(const InputParameters & parameters);

  virtual ~InitialConditionBase();

  /**
   * retrieves the MOOSE variable that this initial condition acts upon
   */
  virtual MooseVariableFEBase & variable() = 0;

  /**
   * getter method for dependent user objects
   */
  const std::set<std::string> & getDependObjects() const { return _depend_uo; }

  /**
   * Workhorse method for projecting the initial conditions for block initial conditions
   */
  virtual void compute() = 0;
  /**
   * Workhorse method for projecting the initial conditions for boundary restricted initial
   * conditions
   */
  virtual void computeNodal(const Point & p) = 0;

  /**
   * Gets called at the beginning of the simulation before this object is asked to do its job.
   * Note: This method is normally inherited from SetupInterface.  However in this case it makes
   * no sense to inherit the other virtuals available in that class so we are adding this virtual
   * directly to this class with out the extra inheritance.
   */
  virtual void initialSetup() {}

  virtual const std::set<std::string> & getRequestedItems() override;

  virtual const std::set<std::string> & getSuppliedItems() override;

  /**
   * reimplements the getUserObject method from UserObjectInterface
   */
  template <typename T2>
  const T2 & getUserObjectTempl(const std::string & name);
  /**
   * reimplements the getUserObjectByName method from UserObjectInterface
   */
  template <typename T2>
  const T2 & getUserObjectByNameTempl(const UserObjectName & name);

  /**
   * reimplements the getUserObjectBase method from UserObjectInterface
   */
  const UserObject & getUserObjectBase(const std::string & name);

protected:
  /// The system object
  SystemBase & _sys;

  /// Dependent variables
  std::set<std::string> _depend_vars;
  /// Supplied variables
  std::set<std::string> _supplied_vars;

  /// Depend UserObjects
  std::set<std::string> _depend_uo;

  /// If set, UOs retrieved by this IC will not be executed before this IC
  const bool _ignore_uo_dependency;
};

template <typename T>
const T &
InitialConditionBase::getUserObjectTempl(const std::string & name)
{
  if (!_ignore_uo_dependency)
    _depend_uo.insert(_pars.get<UserObjectName>(name));
  return UserObjectInterface::getUserObjectTempl<T>(name);
}

template <typename T>
const T &
InitialConditionBase::getUserObjectByNameTempl(const UserObjectName & name)
{
  if (!_ignore_uo_dependency)
    _depend_uo.insert(name);
  return UserObjectInterface::getUserObjectByNameTempl<T>(name);
}
