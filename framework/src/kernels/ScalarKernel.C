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

  params.addParam<bool>("use_displaced_mesh", false, "Whether or not this object should use the displaced mesh for computation.  Note that in the case this is true but no displacements are provided in the Mesh block the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.registerBase("ScalarKernel");

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
    ZeroInterface(parameters),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _sys(*parameters.get<SystemBase *>("_sys")),

    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _var(_sys.getScalarVariable(_tid, parameters.get<NonlinearVariableName>("variable"))),
    _mesh(_subproblem.mesh()),
//    _dim(_mesh.dimension()),

    _u(_var.sln()),
    _u_old(_var.slnOld()),
    _u_dot(_var.uDot()),
    _du_dot_du(_var.duDotDu())
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
