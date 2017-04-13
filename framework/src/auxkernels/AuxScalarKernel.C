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

template <>
InputParameters
validParams<AuxScalarKernel>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<SetupInterface>();
  params += validParams<MeshChangedInterface>();

  params.addRequiredParam<AuxVariableName>("variable",
                                           "The name of the variable that this kernel operates on");
  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation.  Note that "
                        "in the case this is true but no "
                        "displacements are provided in the Mesh block "
                        "the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.declareControllable("enable"); // allows Control to enable/disable this type of object
  params.registerBase("AuxScalarKernel");

  return params;
}

AuxScalarKernel::AuxScalarKernel(const InputParameters & parameters)
  : MooseObject(parameters),
    ScalarCoupleable(this),
    SetupInterface(this),
    FunctionInterface(this),
    UserObjectInterface(this),
    PostprocessorInterface(this),
    DependencyResolverInterface(),
    TransientInterface(this),
    ZeroInterface(parameters),
    MeshChangedInterface(parameters),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _var(_sys.getScalarVariable(_tid, parameters.get<AuxVariableName>("variable"))),
    _mesh(_subproblem.mesh()),
    _u(_var.sln()),
    _u_old(_var.slnOld())
{
  _supplied_vars.insert(parameters.get<AuxVariableName>("variable"));

  const std::vector<MooseVariableScalar *> & coupled_vars = getCoupledMooseScalarVars();
  for (const auto & var : coupled_vars)
    _depend_vars.insert(var->name());
}

AuxScalarKernel::~AuxScalarKernel() {}

void
AuxScalarKernel::compute()
{
  for (_i = 0; _i < _var.order(); ++_i)
  {
    Real value = computeValue();
    _var.setValue(_i, value); // update variable data, which is referenced by other kernels, so the
                              // value is up-to-date
  }
}

const std::set<std::string> &
AuxScalarKernel::getRequestedItems()
{
  return _depend_vars;
}

const std::set<std::string> &
AuxScalarKernel::getSuppliedItems()
{
  return _supplied_vars;
}

bool
AuxScalarKernel::isActive()
{
  return true;
}
