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

#include "Marker.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "Assembly.h"
#include "MooseVariable.h"


template<>
InputParameters validParams<Marker>()
{
  InputParameters params = validParams<MooseObject>();
  params.addParam<std::vector<SubdomainName> >("block", "The block id where this object lives.");

  params.addPrivateParam<bool>("use_displaced_mesh", false);
  params.addPrivateParam<std::string>("built_by_action", "add_marker");
  return params;
}

Marker::Marker(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    SetupInterface(parameters),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _fe_problem(*parameters.get<FEProblem *>("_fe_problem")),
    _adaptivity(_fe_problem.adaptivity()),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),

    _field_var(_sys.getVariable(_tid, name)),
    _current_elem(_field_var.currentElem()),

    _mesh(_subproblem.mesh())
{
  _supplied.insert(name);
}

MooseEnum
Marker::markerStates()
{
  // MooseEnum marker_states("COARSEN = 0, DO_NOTHING, REFINE, JUST_REFINED, JUST_COARSENED, INACTIVE, COARSEN_INACTIVE, INVALID_REFINEMENTSTATE");
  MooseEnum marker_states("dont_mark = -1, coarsen, do_nothing, refine");

  return marker_states;
}

void
Marker::computeMarker()
{
  int mark = computeElementMarker();
  _field_var.setNodalValue(mark);
}

ErrorVector &
Marker::getErrorVector(std::string indicator)
{
  return _adaptivity.getErrorVector(indicator);
}

VariableValue &
Marker::getMarkerValue(std::string name)
{
  _depend.insert(name);
  return _sys.getVariable(_tid, name).nodalSln();
}
