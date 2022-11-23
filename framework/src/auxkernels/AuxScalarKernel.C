//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AuxScalarKernel.h"
#include "Assembly.h"
#include "MooseVariableScalar.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"

InputParameters
AuxScalarKernel::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += SetupInterface::validParams();
  params += MeshChangedInterface::validParams();

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
    MeshChangedInterface(parameters),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid, 0)),
    _var(_sys.getScalarVariable(_tid, parameters.get<AuxVariableName>("variable"))),
    _mesh(_subproblem.mesh()),
    _u(_var.sln())
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
  // In general, we want to compute AuxScalarKernel values
  // redundantly, on every processor, to avoid communication.
  //
  // However, in rare cases not all processors will have access to a
  // particular scalar variable, in which case we skip computation
  // there.
  if (_var.dofIndices().empty() || !_var.dofMap().all_semilocal_indices(_var.dofIndices()))
    return;

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

const VariableValue &
AuxScalarKernel::uOld() const
{
  if (_sys.solutionStatesInitialized())
    mooseError("The solution states have already been initialized when calling ",
               type(),
               "::uOld().\n\n",
               "Make sure to call uOld() within the object constructor.");

  return _var.slnOld();
}
