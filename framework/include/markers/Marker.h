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

class MooseMesh;
class Problem;
class SubProblem;
class SystemBase;
class Assembly;
class MooseVariable;
class Marker;

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

  SubProblem & _subproblem;
  FEProblem & _fe_problem;
  SystemBase & _sys;

  THREAD_ID _tid;

  Assembly & _assembly;

  MooseVariable & _field_var;

  MooseMesh & _mesh;
};

#endif /* MARKER_H */
