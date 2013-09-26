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
  params += validParams<BlockRestrictable>();

  params.addParam<bool>("use_displaced_mesh", false, "Whether or not this object should use the displaced mesh for computation.  Note that in the case this is true but no displacements are provided in the Mesh block the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");
  params.addPrivateParam<std::string>("built_by_action", "add_marker");
  return params;
}

Marker::Marker(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    BlockRestrictable(parameters),
    SetupInterface(parameters),
    UserObjectInterface(parameters),
    Restartable(name, parameters, "Markers"),
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

  addMooseVariableDependency(&_field_var);
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

bool
Marker::isActive() const
{
  return true;
}

void
Marker::markerSetup()
{
}

const std::set<std::string> &
Marker::getRequestedItems()
{
  return _depend;
}

const std::set<std::string> &
Marker::getSuppliedItems()
{
  return _supplied;
}
