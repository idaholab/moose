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

#ifndef INITIALCONDITION_H
#define INITIALCONDITION_H

#include "MooseObject.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "ParallelUniqueId.h"
#include "Restartable.h"
#include "BlockRestrictable.h"
#include "DependencyResolverInterface.h"
#include "BoundaryRestrictable.h"
#include "Reportable.h"
#include "ZeroInterface.h"
// System includes
#include <string>

// libMesh
#include "libmesh/point.h"
#include "libmesh/vector_value.h"
#include "libmesh/elem.h"

//forward declarations
class InitialCondition;
class FEProblem;
class SystemBase;
class Assembly;
class MooseVariable;

template<>
InputParameters validParams<InitialCondition>();

/**
 * InitialConditions are objects that set the initial value of variables.
 */
class InitialCondition :
  public MooseObject,
  public Coupleable,
  public FunctionInterface,
  public UserObjectInterface,
  public BlockRestrictable,
  public BoundaryRestrictable,
  public DependencyResolverInterface,
  public Restartable,
  public Reportable,
  public ZeroInterface
{
public:
  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   */
  InitialCondition(const std::string & name, InputParameters parameters);

  virtual ~InitialCondition();

  MooseVariable & variable() { return _var; }

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

  virtual const std::set<std::string> & getRequestedItems();

  virtual const std::set<std::string> & getSuppliedItems();

protected:
  FEProblem & _fe_problem;
  SystemBase & _sys;
  THREAD_ID _tid;

  Assembly & _assembly;

  /// Time
  Real & _t;

  const Moose::CoordinateSystemType & _coord_sys;
  MooseVariable & _var;

  const Elem * & _current_elem;

  unsigned int _qp;

  std::set<std::string> _depend_vars;
  std::set<std::string> _supplied_vars;
};

#endif //INITIALCONDITION_H
