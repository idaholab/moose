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
#include "MooseVariableFE.h"
#include "Assembly.h"

defineLegacyParams(NodalKernel);

InputParameters
NodalKernel::validParams()
{
  InputParameters params = ResidualObject::validParams();
  params += BlockRestrictable::validParams();
  params += BoundaryRestrictable::validParams();

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

  params.registerBase("NodalKernel");

  return params;
}

NodalKernel::NodalKernel(const InputParameters & parameters)
  : ResidualObject(parameters, true),
    BlockRestrictable(this),
    BoundaryRestrictable(this, true), // true for applying to nodesets
    GeometricSearchInterface(this),
    CoupleableMooseVariableDependencyIntermediateInterface(this, true),
    MooseVariableInterface<Real>(this,
                                 true,
                                 "variable",
                                 Moose::VarKindType::VAR_NONLINEAR,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _var(*mooseVariable()),
    _current_node(_var.node()),
    _u(_var.dofValues()),
    _save_in_strings(parameters.get<std::vector<AuxVariableName>>("save_in")),
    _diag_save_in_strings(parameters.get<std::vector<AuxVariableName>>("diag_save_in"))

{
  _save_in.resize(_save_in_strings.size());
  _diag_save_in.resize(_diag_save_in_strings.size());

  for (unsigned int i = 0; i < _save_in_strings.size(); i++)
  {
    MooseVariable * var = &_subproblem.getStandardVariable(_tid, _save_in_strings[i]);

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
    MooseVariable * var = &_subproblem.getStandardVariable(_tid, _diag_save_in_strings[i]);

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

void
NodalKernel::computeResidual()
{
  if (_var.isNodalDefined())
  {
    const dof_id_type & dof_idx = _var.nodalDofIndex();
    _qp = 0;
    Real res = computeQpResidual();
    res *= _var.scalingFactor();
    _assembly.cacheResidual(dof_idx, res, _vector_tags);

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

    cached_val *= _var.scalingFactor();

    _assembly.cacheJacobian(cached_row, cached_row, cached_val, _matrix_tags);

    if (_has_diag_save_in)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (const auto & var : _diag_save_in)
        var->sys().solution().add(var->nodalDofIndex(), cached_val);
    }
  }
}

void
NodalKernel::computeOffDiagJacobian(const unsigned int jvar_num)
{
  const auto & jvar = getVariable(jvar_num);

  if (_var.isNodalDefined())
  {
    if (jvar.number() == _var.number())
      computeJacobian();
    else
    {
      _qp = 0;
      Real cached_val = computeQpOffDiagJacobian(jvar.number());
      dof_id_type cached_row = _var.nodalDofIndex();

      // Note: this only works for equal order Lagrange variables...
      dof_id_type cached_col = _current_node->dof_number(_sys.number(), jvar.number(), 0);

      cached_val *= _var.scalingFactor();

      _assembly.cacheJacobian(cached_row, cached_col, cached_val, _matrix_tags);
    }
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
