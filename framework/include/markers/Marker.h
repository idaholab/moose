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

#ifndef MARKER_H
#define MARKER_H

#include "MooseObject.h"
#include "MooseVariableInterface.h"
#include "InputParameters.h"
#include "SetupInterface.h"

// libmesh Includes
#include "threads.h"
#include "error_vector.h"

class MooseMesh;
class Problem;
class SubProblem;
class SystemBase;
class Assembly;
class MooseVariable;
class Marker;
class Adaptivity;

template<>
InputParameters validParams<Marker>();

class Marker :
  public MooseObject,
  public SetupInterface
{
public:
  Marker(const std::string & name, InputParameters parameters);
  virtual ~Marker(){}

  /**
   * Helper function for getting the valid refinement flag states a marker can use as a MooseEnum.
   * @return A MooseEnum that is filled with the valid states.  These are perfectly transferable to libMesh Elem::RefinementStates.
   */
  static MooseEnum markerStates();

  /**
   * Create an Error Vector from the Marker field
   * TODO: Fix this return type
   */
  void getErrorVector() {}

  virtual void computeMarker();

  // TODO: Fixme
  const bool isActive() { return true; }

  virtual void markerSetup() {}

protected:

  virtual int computeElementMarker() = 0;

  /**
   * Get an ErrorVector that will be filled up with values corresponding to the
   * indicator field name passed in.
   *
   * Note that this returns a reference... and the return value should be stored as a reference!
   *
   * @param indicator_field The name of the field to get an ErrorVector for.
   */
  ErrorVector & getErrorVector(std::string indicator_field);

  SubProblem & _subproblem;
  FEProblem & _fe_problem;
  Adaptivity & _adaptivity;
  SystemBase & _sys;

  THREAD_ID _tid;

  Assembly & _assembly;

  MooseVariable & _field_var;
  const Elem * & _current_elem;

  MooseMesh & _mesh;
};

#endif /* MARKER_H */
