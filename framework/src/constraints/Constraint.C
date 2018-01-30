//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Constraint.h"

#include "SystemBase.h"

template <>
InputParameters
validParams<Constraint>()
{
  InputParameters params = validParams<MooseObject>();
  // Add the SetupInterface parameter, 'execute_on', default is 'linear'
  params += validParams<SetupInterface>();

  params.addRequiredParam<NonlinearVariableName>(
      "variable", "The name of the variable that this constraint is applied to.");
  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation.  Note that "
                        "in the case this is true but no "
                        "displacements are provided in the Mesh block "
                        "the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.declareControllable("enable");
  params.registerBase("Constraint");

  return params;
}

Constraint::Constraint(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    FunctionInterface(this),
    UserObjectInterface(this),
    TransientInterface(this),
    GeometricSearchInterface(this),
    Restartable(parameters, "Constraints"),
    MeshChangedInterface(parameters),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _var(_sys.getVariable(_tid, parameters.get<NonlinearVariableName>("variable"))),
    _mesh(_subproblem.mesh())
{
}

Constraint::~Constraint() {}
