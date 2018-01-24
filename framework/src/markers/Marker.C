//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Marker.h"

#include "Assembly.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SystemBase.h"

template <>
InputParameters
validParams<Marker>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<BlockRestrictable>();
  params += validParams<OutputInterface>();

  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation.  Note that "
                        "in the case this is true but no "
                        "displacements are provided in the Mesh block "
                        "the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.registerBase("Marker");

  return params;
}

Marker::Marker(const InputParameters & parameters)
  : MooseObject(parameters),
    BlockRestrictable(this),
    SetupInterface(this),
    DependencyResolverInterface(),
    UserObjectInterface(this),
    Restartable(parameters, "Markers"),
    PostprocessorInterface(this),
    MeshChangedInterface(parameters),
    OutputInterface(parameters),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _adaptivity(_fe_problem.adaptivity()),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
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
