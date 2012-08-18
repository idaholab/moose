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

// libmesh includes
#include "threads.h"

FlagElementsThread::FlagElementsThread(FEProblem & fe_problem, std::vector<Number> & serialized_solution) :
    ThreadedElementLoop<ConstElemRange>(fe_problem, fe_problem.getAuxiliarySystem()),
    _fe_problem(fe_problem),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    _system_number(_aux_sys.number()),
    _adaptivity(_fe_problem.adaptivity()),
    _field_var(_adaptivity.getMarkerVariable()),
    _field_var_number(_field_var.number()),
    _serialized_solution(serialized_solution)
{
}

// Splitting Constructor
FlagElementsThread::FlagElementsThread(FlagElementsThread & x, Threads::split split) :
    ThreadedElementLoop<ConstElemRange>(x, split),
    _fe_problem(x._fe_problem),
    _aux_sys(x._aux_sys),
    _system_number(x._system_number),
    _adaptivity(x._adaptivity),
    _field_var(x._field_var),
    _field_var_number(x._field_var_number),
    _serialized_solution(x._serialized_solution)
{
}

void
FlagElementsThread::onElement(const Elem *elem)
{
  unsigned int dof_number = elem->dof_number(_system_number, _field_var_number, 0);
  Marker::MarkerValue marker_value = (Marker::MarkerValue)_serialized_solution[dof_number];

  // If no Markers cared about what happened to this element let's just leave it alone
  if(marker_value == Marker::DONT_MARK)
    marker_value = Marker::DO_NOTHING;

  const_cast<Elem *>(elem)->set_refinement_flag((Elem::RefinementState)marker_value);
}

void
FlagElementsThread::join(const FlagElementsThread & /*y*/)
{
}
