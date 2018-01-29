//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalKernel.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseVariable.h"
#include "Assembly.h"

template <>
InputParameters
validParams<NodalKernel>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<TransientInterface>();
  params += validParams<BlockRestrictable>();
  params += validParams<BoundaryRestrictable>();
  params += validParams<RandomInterface>();

  params.addRequiredParam<NonlinearVariableName>(
      "variable", "The name of the variable that this boundary condition applies to");

  params.addParam<std::vector<AuxVariableName>>(
      "save_in",
      "The name of auxiliary variables to save this BC's residual contributions to.  "
      "Everything about that variable must match everything about this variable (the "
      "type, what blocks it's on, etc.)");

  params.addParam<std::vector<AuxVariableName>>(
      "diag_save_in",
      "The name of auxiliary variables to save this BC's diagonal jacobian "
      "contributions to.  Everything about that variable must match everything "
      "about this variable (the type, what blocks it's on, etc.)");

  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation.  Note that "
                        "in the case this is true but no "
                        "displacements are provided in the Mesh block "
                        "the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.declareControllable("enable");

  params.registerBase("NodalKernel");

  return params;
}

NodalKernel::NodalKernel(const InputParameters & parameters)
  : MooseObject(parameters),
    BlockRestrictable(this),
    BoundaryRestrictable(this, true), // true for applying to nodesets
    SetupInterface(this),
    FunctionInterface(this),
    UserObjectInterface(this),
    TransientInterface(this),
    PostprocessorInterface(this),
    GeometricSearchInterface(this),
    Restartable(parameters, "BCs"),
    MeshChangedInterface(parameters),
    RandomInterface(parameters,
                    *parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"),
                    parameters.get<THREAD_ID>("_tid"),
                    true),
    CoupleableMooseVariableDependencyIntermediateInterface(this, true),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _var(_sys.getVariable(_tid, parameters.get<NonlinearVariableName>("variable"))),
    _mesh(_subproblem.mesh()),
    _current_node(_var.node()),
    _u(_var.nodalSln()),
    _u_dot(_var.nodalSlnDot()),
    _du_dot_du(_var.nodalSlnDuDotDu()),
    _save_in_strings(parameters.get<std::vector<AuxVariableName>>("save_in")),
    _diag_save_in_strings(parameters.get<std::vector<AuxVariableName>>("diag_save_in"))

{
  _save_in.resize(_save_in_strings.size());
  _diag_save_in.resize(_diag_save_in_strings.size());

  for (unsigned int i = 0; i < _save_in_strings.size(); i++)
  {
    MooseVariable * var = &_subproblem.getVariable(_tid, _save_in_strings[i]);

    if (var->feType() != _var.feType())
      paramError(
          "save_in",
          "saved-in auxiliary variable is incompatible with the object's nonlinear variable: ",
          moose::internal::incompatVarMsg(*var, _var));

    _save_in[i] = var;
    var->sys().addVariableToZeroOnResidual(_save_in_strings[i]);
    addMooseVariableDependency(var);
  }

  _has_save_in = _save_in.size() > 0;

  for (unsigned int i = 0; i < _diag_save_in_strings.size(); i++)
  {
    MooseVariable * var = &_subproblem.getVariable(_tid, _diag_save_in_strings[i]);

    if (var->feType() != _var.feType())
      paramError(
          "diag_save_in",
          "saved-in auxiliary variable is incompatible with the object's nonlinear variable: ",
          moose::internal::incompatVarMsg(*var, _var));

    _diag_save_in[i] = var;
    var->sys().addVariableToZeroOnJacobian(_diag_save_in_strings[i]);
    addMooseVariableDependency(var);
  }

  _has_diag_save_in = _diag_save_in.size() > 0;
}

MooseVariable &
NodalKernel::variable()
{
  return _var;
}

SubProblem &
NodalKernel::subProblem()
{
  return _subproblem;
}

void
NodalKernel::computeResidual()
{
  if (_var.isNodalDefined())
  {
    dof_id_type & dof_idx = _var.nodalDofIndex();
    _qp = 0;
    Real res = computeQpResidual();
    _assembly.cacheResidualContribution(dof_idx, res, Moose::KT_NONTIME);

    if (_has_save_in)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (const auto & var : _save_in)
        var->sys().solution().add(var->nodalDofIndex(), res);
    }
  }
}

void
NodalKernel::computeJacobian()
{
  if (_var.isNodalDefined())
  {
    _qp = 0;
    Real cached_val = computeQpJacobian();
    dof_id_type cached_row = _var.nodalDofIndex();

    _assembly.cacheJacobianContribution(cached_row, cached_row, cached_val);

    if (_has_diag_save_in)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (const auto & var : _diag_save_in)
        var->sys().solution().add(var->nodalDofIndex(), cached_val);
    }
  }
}

void
NodalKernel::computeOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var.number())
    computeJacobian();
  else
  {
    _qp = 0;
    Real cached_val = computeQpOffDiagJacobian(jvar);
    dof_id_type cached_row = _var.nodalDofIndex();
    // Note: this only works for Lagrange variables...
    dof_id_type cached_col = _current_node->dof_number(_sys.number(), jvar, 0);

    _assembly.cacheJacobianContribution(cached_row, cached_col, cached_val);
  }
}

Real
NodalKernel::computeQpJacobian()
{
  return 0.;
}

Real
NodalKernel::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.;
}
