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
  params += validParams<OutputInterface>();

  params.addParam<bool>("use_displaced_mesh", false, "Whether or not this object should use the displaced mesh for computation.  Note that in the case this is true but no displacements are provided in the Mesh block the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.registerBase("Marker");

  return params;
}

Marker::Marker(const InputParameters & parameters) :
    MooseObject(parameters),
    BlockRestrictable(parameters),
    SetupInterface(this),
    DependencyResolverInterface(),
    MooseVariableDependencyInterface(this),
    UserObjectInterface(this),
    Restartable(parameters, "Markers"),
    PostprocessorInterface(this),
    MeshChangedInterface(parameters),
    OutputInterface(parameters),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _fe_problem(*parameters.get<FEProblemBase *>("_fe_problem_base")),
    _adaptivity(_fe_problem.adaptivity()),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _field_var(_sys.getVariable(_tid, name())),
    _current_elem(_field_var.currentElem()),

    _mesh(_subproblem.mesh())
{
  _supplied.insert(name());

  addMooseVariableDependency(&_field_var);
}

MooseEnum
Marker::markerStates()
{
  MooseEnum marker_states("DONT_MARK=-1 COARSEN DO_NOTHING REFINE");

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

const VariableValue &
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
