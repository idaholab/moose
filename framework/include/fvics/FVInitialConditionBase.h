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
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "PostprocessorInterface.h"
#include "Restartable.h"
#include "BlockRestrictable.h"
#include "DependencyResolverInterface.h"
#include "MooseTypes.h"
#include "NonADFunctorInterface.h"

class SystemBase;
class MooseVariableFieldBase;
namespace libMesh
{
class Point;
}

/**
 * description
 */
class FVInitialConditionBase : public MooseObject,
                               public InitialConditionInterface,
                               public BlockRestrictable,
                               public FunctionInterface,
                               public Restartable,
                               public DependencyResolverInterface,
                               public NonADFunctorInterface
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  FVInitialConditionBase(const InputParameters & parameters);

  virtual ~FVInitialConditionBase();

  static InputParameters validParams();

  /**
   * retrieves the MOOSE variable that this initial condition acts upon
   */
  virtual MooseVariableFieldBase & variable() = 0;

  /**
   * Workhorse method for computing the initial conditions for block-restricted initial
   * conditions
   */
  virtual void computeElement(const ElemInfo & elem_info) = 0;

  /**
   * Gets called at the beginning of the simulation before this object is asked to do its job.
   * Note: This method is normally inherited from SetupInterface.  However in this case it makes
   * no sense to inherit the other virtuals available in that class so we are adding this virtual
   * directly to this class without the extra inheritance.
   */
  virtual void initialSetup() {}

  virtual const std::set<std::string> & getRequestedItems() override { return _depend_vars; }

  virtual const std::set<std::string> & getSuppliedItems() override { return _supplied_vars; }

protected:
  /// The system object
  SystemBase & _sys;

private:
  /// Dependent variables
  std::set<std::string> _depend_vars;
  /// Supplied variables
  std::set<std::string> _supplied_vars;
};
