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

#include "ScalarKernel.h"
#include "Assembly.h"
#include "MooseVariableScalar.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "NonlinearSystem.h"

template<>
InputParameters validParams<ScalarKernel>()
{
  InputParameters params = validParams<MooseObject>();
  params.addRequiredParam<NonlinearVariableName>("variable", "The name of the variable that this kernel operates on");

  params.addPrivateParam<bool>("use_displaced_mesh", false);
  params.addPrivateParam<std::string>("built_by_action", "add_scalar_kernel");

  return params;
}


ScalarKernel::ScalarKernel(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    ScalarCoupleable(parameters),
    SetupInterface(parameters),
    FunctionInterface(parameters),
    UserObjectInterface(parameters),
    PostprocessorInterface(parameters),
    TransientInterface(parameters, name, "scalar_kernel"),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _sys(*parameters.get<SystemBase *>("_sys")),

    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _var(_sys.getScalarVariable(_tid, parameters.get<NonlinearVariableName>("variable"))),
    _mesh(_subproblem.mesh()),
    _dim(_mesh.dimension()),

    _u(_var.sln()),
    _u_old(_var.slnOld()),
    _u_dot(_var.uDot()),
    _du_dot_du(_var.duDotDu()),

    _real_zero(_subproblem._real_zero[_tid]),
    _zero(_subproblem._zero[_tid]),
    _grad_zero(_subproblem._grad_zero[_tid]),
    _second_zero(_subproblem._second_zero[_tid])
{
}

void
ScalarKernel::computeOffDiagJacobian(unsigned int /*jvar*/)
{
}

bool
ScalarKernel::isActive()
{
  return true;
}

MooseVariableScalar &
ScalarKernel::variable()
{
  return _var;
}

SubProblem &
ScalarKernel::subProblem()
{
  return _subproblem;
}
