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
#include "ParallelUniqueId.h"

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
  public FunctionInterface,
  public UserObjectInterface
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
};

#endif //INITIALCONDITION_H
