//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlagElementsThread.h"

// MOOSE includes
#include "AuxiliarySystem.h"
#include "DisplacedProblem.h"
#include "FEProblem.h"
#include "Marker.h"
#include "MooseVariableFE.h"
#include "Problem.h"

#include "libmesh/threads.h"

// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)

FlagElementsThread::FlagElementsThread(FEProblemBase & fe_problem,
                                       std::vector<Number> & serialized_solution,
                                       unsigned int max_h_level,
                                       const std::string & marker_name,
                                       bool is_serialized_solution)
  : ThreadedElementLoop<ConstElemRange>(fe_problem),
    _fe_problem(fe_problem),
    _displaced_problem(_fe_problem.getDisplacedProblem()),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    _system_number(_aux_sys.number()),
    _adaptivity(_fe_problem.adaptivity()),
    _field_var(_fe_problem.getVariable(
        0, marker_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY)),
    _field_var_number(_field_var.number()),
    _serialized_solution(serialized_solution),
    _max_h_level(max_h_level),
    _is_serialized_solution(is_serialized_solution)
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
    _max_h_level(x._max_h_level),
    _is_serialized_solution(x._is_serialized_solution)
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

    Number dof_value = 0.;
    // If solution is serialized in the caller,
    // we use the serialized solution
    if (_is_serialized_solution)
      dof_value = _serialized_solution[dof_number];
    else // Otherwise, we look at the ghosted local solution
    {
      // Local ghosted solution
      auto & current_local_solution = *_aux_sys.currentSolution();
      // Libesh will convert a global dof number to a local one,
      // then return the corresponding entry value
      dof_value = current_local_solution(dof_number);
    }

    // round() is a C99 function, it is not located in the std:: namespace.
    marker_value = static_cast<Marker::MarkerValue>(round(dof_value));

    // Make sure we aren't masking an issue in the Marker system by rounding its values.
    if (std::abs(marker_value - dof_value) > TOLERANCE * TOLERANCE)
      mooseError("Invalid Marker value detected: ", dof_value);
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
