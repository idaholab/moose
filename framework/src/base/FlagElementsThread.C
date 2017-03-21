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
#include "FlagElementsThread.h"

#include "AuxiliarySystem.h"
#include "Problem.h"
#include "FEProblem.h"
#include "Marker.h"
#include "DisplacedProblem.h"

// libmesh includes
#include "libmesh/threads.h"

// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)

FlagElementsThread::FlagElementsThread(FEProblemBase & fe_problem,
                                       std::vector<Number> & serialized_solution,
                                       unsigned int max_h_level,
                                       const std::string & marker_name)
  : ThreadedElementLoop<ConstElemRange>(fe_problem),
    _fe_problem(fe_problem),
    _displaced_problem(_fe_problem.getDisplacedProblem()),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    _system_number(_aux_sys.number()),
    _adaptivity(_fe_problem.adaptivity()),
    _field_var(_fe_problem.getVariable(0, marker_name)),
    _field_var_number(_field_var.number()),
    _serialized_solution(serialized_solution),
    _max_h_level(max_h_level)
{
}

// Splitting Constructor
FlagElementsThread::FlagElementsThread(FlagElementsThread & x, Threads::split split)
  : ThreadedElementLoop<ConstElemRange>(x, split),
    _fe_problem(x._fe_problem),
    _displaced_problem(x._displaced_problem),
    _aux_sys(x._aux_sys),
    _system_number(x._system_number),
    _adaptivity(x._adaptivity),
    _field_var(x._field_var),
    _field_var_number(x._field_var_number),
    _serialized_solution(x._serialized_solution),
    _max_h_level(x._max_h_level)
{
}

void
FlagElementsThread::onElement(const Elem * elem)
{
  // By default do nothing, and only grab the marker from the solution if the current variable is
  // active
  // on the element subdomain.
  Marker::MarkerValue marker_value = Marker::DO_NOTHING;
  if (_field_var.activeOnSubdomain(elem->subdomain_id()))
  {
    dof_id_type dof_number = elem->dof_number(_system_number, _field_var_number, 0);

    // round() is a C99 function, it is not located in the std:: namespace.
    marker_value = static_cast<Marker::MarkerValue>(round(_serialized_solution[dof_number]));

    // Make sure we aren't masking an issue in the Marker system by rounding its values.
    if (std::abs(marker_value - _serialized_solution[dof_number]) > TOLERANCE * TOLERANCE)
      mooseError("Invalid Marker value detected: ", _serialized_solution[dof_number]);
  }

  // If no Markers cared about what happened to this element let's just leave it alone
  if (marker_value == Marker::DONT_MARK)
    marker_value = Marker::DO_NOTHING;

  // Don't refine past the max level
  if (_max_h_level && marker_value == Marker::REFINE && elem->level() >= _max_h_level)
    marker_value = Marker::DO_NOTHING;

  const_cast<Elem *>(elem)->set_refinement_flag((Elem::RefinementState)marker_value);

  if (_displaced_problem)
    _displaced_problem->mesh()
        .elemPtr(elem->id())
        ->set_refinement_flag((Elem::RefinementState)marker_value);
}

void
FlagElementsThread::join(const FlagElementsThread & /*y*/)
{
}
