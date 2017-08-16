//****************************************************************/
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

#include "NestedTransient.h"

// MOOSE includes
#include "Factory.h"
#include "SubProblem.h"
#include "TimeStepper.h"
#include "MooseApp.h"
#include "Conversion.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"
#include "Control.h"
#include "TimePeriod.h"

// libMesh includes
#include "libmesh/implicit_system.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/transient_system.h"
#include "libmesh/numeric_vector.h"

// C++ Includes
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

template <>
InputParameters
validParams<NestedTransient>()
{
  InputParameters params = validParams<Transient>();
  params.addParam<unsigned int>(
      "max_xfem_update",
      std::numeric_limits<unsigned int>::max(),
      "Maximum number of times to update XFEM crack topology in a step due to evolving cracks");
  return params;
}

NestedTransient::NestedTransient(const InputParameters & parameters)
  : Transient(parameters),
    _xfem_repeat_step(false),
    _xfem_update_count(0),
    _max_xfem_update(getParam<unsigned int>("max_xfem_update"))
{
}

void
NestedTransient::incrementStepOrReject()
{
  if (lastSolveConverged())
  {
    if (_xfem_repeat_step)
    {
      _time = _time_old;
      _first = false;
      return;
    }
  }

  Transient::incrementStepOrReject();
}

void
NestedTransient::acceptanceCheck(Real current_dt)
{
  if (lastSolveConverged())
  {
    _console << COLOR_GREEN << " Solve Converged!" << COLOR_DEFAULT << std::endl;

    if (_problem.haveXFEM())
    {
      if (_problem.updateMeshXFEM() && (_xfem_update_count < _max_xfem_update))
      {
        _console << "XFEM modifying mesh, repeating step" << std::endl;
        _xfem_repeat_step = true;
        ++_xfem_update_count;
        Transient::convergePostSolve(current_dt);
      }
      else
      {
        _xfem_repeat_step = false;
        _xfem_update_count = 0;
        _console << "XFEM not modifying mesh, continuing" << std::endl;
        Transient::acceptStep();
        Transient::convergePostSolve(current_dt);
      }
    }
    else
    {
      Transient::acceptStep();
      Transient::convergePostSolve(current_dt);
    }
  }
  else
  {
    Transient::rejectStep();
    Transient::divergePostSolve(current_dt);
  }
}

void
NestedTransient::endStep(Real input_time)
{
  if (input_time == -1.0)
    _time = _time_old + _dt;
  else
    _time = input_time;

  _picard_converged = false;

  if (_xfem_repeat_step)
    return;

  Transient::endStep(input_time);
}

bool
NestedTransient::keepGoing()
{
  bool keep_going = !_problem.isSolveTerminationRequested();

  if (_xfem_repeat_step)
    return Transient::stopCondition(keep_going);

  // Check for stop condition based upon steady-state check flag:
  return Transient::keepGoing();
}
