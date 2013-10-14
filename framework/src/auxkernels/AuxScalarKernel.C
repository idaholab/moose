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

#include "AuxScalarKernel.h"
#include "Assembly.h"
#include "MooseVariableScalar.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"

template<>
InputParameters validParams<AuxScalarKernel>()
{
  InputParameters params = validParams<MooseObject>();
  params.addRequiredParam<AuxVariableName>("variable", "The name of the variable that this kernel operates on");
  params.addParam<bool>("use_displaced_mesh", false, "Whether or not this object should use the displaced mesh for computation.  Note that in the case this is true but no displacements are provided in the Mesh block the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");
  params.addPrivateParam<std::string>("built_by_action", "add_aux_scalar_kernel");

  return params;
}

AuxScalarKernel::AuxScalarKernel(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    ScalarCoupleable(parameters),
    SetupInterface(parameters),
    FunctionInterface(parameters),
    UserObjectInterface(parameters),
    PostprocessorInterface(parameters),
    TransientInterface(parameters, name, "scalar_aux_kernels"),
    Reportable(name, parameters),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _sys(*parameters.get<SystemBase *>("_sys")),

    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _var(_sys.getScalarVariable(_tid, parameters.get<AuxVariableName>("variable"))),
    _mesh(_subproblem.mesh()),
    _dim(_mesh.dimension()),

    _u(_var.sln()),
    _u_old(_var.slnOld()),

    _real_zero(_subproblem._real_zero[_tid]),
    _zero(_subproblem._zero[_tid]),
    _grad_zero(_subproblem._grad_zero[_tid]),
    _second_zero(_subproblem._second_zero[_tid])
{
}

AuxScalarKernel::~AuxScalarKernel()
{
}

void
AuxScalarKernel::compute()
{
  for (_i = 0; _i < _var.order(); ++_i)
  {
    Real value = computeValue();
    _var.setValue(_i, value);                  // update variable data, which is referenced by other kernels, so the value is up-to-date
  }
}

bool
AuxScalarKernel::isActive()
{
  return true;
}
