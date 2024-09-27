//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialConditionInterface.h"
#include "MooseObject.h"
#include "Coupleable.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "PostprocessorInterface.h"
#include "Restartable.h"
#include "BlockRestrictable.h"
#include "DependencyResolverInterface.h"
#include "BoundaryRestrictable.h"
#include "MaterialPropertyInterface.h"
#include "MooseTypes.h"
#include "ElementIDInterface.h"

class SystemBase;
class MooseVariableFieldBase;
namespace libMesh
{
class Point;
}

/**
 * InitialConditionBase serves as the abstract base class for InitialConditions and
 * VectorInitialConditions. Implements methods for getting user objects and dependent objects. The
 * template class that inherits from this class implements the meat of the initial condition
 * hierarchy: the `compute` method
 */
class InitialConditionBase : public MooseObject,
                             public InitialConditionInterface,
                             public BlockRestrictable,
                             public Coupleable,
                             public MaterialPropertyInterface,
                             public FunctionInterface,
                             public UserObjectInterface,
                             public PostprocessorInterface,
                             public BoundaryRestrictable,
                             public DependencyResolverInterface,
                             public Restartable,
                             public ElementIDInterface
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  InitialConditionBase(const InputParameters & parameters);

  virtual ~InitialConditionBase();

  static InputParameters validParams();

  /**
   * retrieves the MOOSE variable that this initial condition acts upon
   */
  virtual MooseVariableFEBase & variable() = 0;

  /**
   * getter method for dependent user objects
   */
  const std::set<UserObjectName> & getDependObjects() const { return _depend_uo; }

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

protected:
  /// The system object
  SystemBase & _sys;

  /// If set, UOs retrieved by this IC will not be executed before this IC
  const bool _ignore_uo_dependency;

private:
  void addUserObjectDependencyHelper(const UserObject & uo) const override final;
  void addPostprocessorDependencyHelper(const PostprocessorName & name) const override final;

  /// Dependent variables
  std::set<std::string> _depend_vars;
  /// Supplied variables
  std::set<std::string> _supplied_vars;

  /// Depend UserObjects. Mutable so that the getters can be const and still add dependencies
  mutable std::set<UserObjectName> _depend_uo;
};
