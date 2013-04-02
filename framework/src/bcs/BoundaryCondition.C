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

#include "BoundaryCondition.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseVariable.h"

template<>
InputParameters validParams<BoundaryCondition>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<TransientInterface>();
  params.addRequiredParam<NonlinearVariableName>("variable", "The name of the variable that this boundary condition applies to");
  params.addPrivateParam<bool>("use_displaced_mesh", false);
  params.addRequiredParam<std::vector<BoundaryName> >("boundary", "The list of boundary IDs from the mesh where this boundary condition applies");

  params.addPrivateParam<std::string>("built_by_action", "add_bc");
  return params;
}


BoundaryCondition::BoundaryCondition(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    SetupInterface(parameters),
    FunctionInterface(parameters),
    UserObjectInterface(parameters),
    TransientInterface(parameters, name, "bcs"),
    PostprocessorInterface(parameters),
    GeometricSearchInterface(parameters),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _fe_problem(*parameters.get<FEProblem *>("_fe_problem")),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _var(_sys.getVariable(_tid, parameters.get<NonlinearVariableName>("variable"))),
    _mesh(_subproblem.mesh()),
    _dim(_mesh.dimension()),
    _boundary_id(parameters.get<BoundaryID>("_boundary_id")),

    _real_zero(_subproblem._real_zero[_tid]),
    _zero(_subproblem._zero[_tid]),
    _grad_zero(_subproblem._grad_zero[_tid]),
    _second_zero(_subproblem._second_zero[_tid])
{
}

bool
BoundaryCondition::shouldApply()
{
  return true;
}
