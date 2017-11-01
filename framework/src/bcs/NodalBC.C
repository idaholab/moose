//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalBC.h"

#include "Assembly.h"
#include "MooseVariableFEImpl.h"
#include "SystemBase.h"
#include "NonlinearSystemBase.h"

template <>
InputParameters
validParams<NodalBC>()
{
  InputParameters params = validParams<NodalBCBase>();
  params += validParams<RandomInterface>();
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

  // These are the default names for tags, but users will be able to add their own
  MultiMooseEnum vtags("nontime time", "nontime", true);
  MultiMooseEnum mtags("nontime time", "nontime", true);

  params.addParam<MultiMooseEnum>(
      "vector_tags", vtags, "The tag for the vectors this Kernel should fill");

  params.addParam<MultiMooseEnum>(
      "matrix_tags", mtags, "The tag for the matrices this Kernel should fill");

  params.addParamNamesToGroup("vector_tags matrix_tags", "Advanced");

  return params;
}

NodalBC::NodalBC(const InputParameters & parameters)
  : NodalBCBase(parameters),
    MooseVariableInterface<Real>(this, true),
    _var(*mooseVariable()),
    _current_node(_var.node()),
    _u(_var.dofValues())
{
  addMooseVariableDependency(mooseVariable());

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

  auto & vector_tag_names = getParam<MultiMooseEnum>("vector_tags");

  if (!vector_tag_names.isValid())
    mooseError("MUST provide at least one vector_tag for Kernel: ", name());

  for (auto & vector_tag_name : vector_tag_names)
  {
    if (!_fe_problem.vectorTagExists(vector_tag_name.name()))
      mooseError("Kernel, ",
                 name(),
                 ", was assigned an invalid vector_tag: '",
                 vector_tag_name,
                 "'.  If this is a TimeKernel then this may have happened because you didn't "
                 "specify a Transient Executioner.");

    _vector_tags.push_back(_fe_problem.getVectorTag(vector_tag_name));
  }

  auto & matrix_tag_names = getParam<MultiMooseEnum>("matrix_tags");

  if (!matrix_tag_names.isValid())
    mooseError("MUST provide at least one matrix_tag for Kernel: ", name());

  for (auto & matrix_tag_name : matrix_tag_names)
  {
    if (!_fe_problem.matrixTagExists(matrix_tag_name))
      mooseError("Kernel, ",
                 name(),
                 ", was assigned an invalid matrix_tag: '",
                 matrix_tag_name,
                 "'.  If this is a TimeKernel then this may have happened because you didn't "
                 "specify a Transient Executioner.");

    _matrix_tags.push_back(_fe_problem.getMatrixTag(matrix_tag_name));
  }
}

void
NodalBC::computeResidual(NumericVector<Number> & residual)
{
  if (_var.isNodalDefined())
  {
    dof_id_type & dof_idx = _var.nodalDofIndex();
    _qp = 0;
    Real res = 0;

    if (!_is_eigen)
      res = computeQpResidual();

    residual.set(dof_idx, res);

    for (auto tag_id : _vector_tags)
      _fe_problem.getNonlinearSystemBase().getVector(tag_id).set(dof_idx, res);

    if (_has_save_in)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (unsigned int i = 0; i < _save_in.size(); i++)
        _save_in[i]->sys().solution().set(_save_in[i]->nodalDofIndex(), res);
    }
  }
}

void
NodalBC::computeJacobian()
{
  // We call the user's computeQpJacobian() function and store the
  // results in the _assembly object. We can't store them directly in
  // the element stiffness matrix, as they will only be inserted after
  // all the assembly is done.
  if (_var.isNodalDefined())
  {
    _qp = 0;
    Real cached_val = computeQpJacobian();
    dof_id_type cached_row = _var.nodalDofIndex();

    // Cache the user's computeQpJacobian() value for later use.
    _fe_problem.assembly(0).cacheJacobianContribution(cached_row, cached_row, cached_val);

    if (_has_diag_save_in)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (unsigned int i = 0; i < _diag_save_in.size(); i++)
        _diag_save_in[i]->sys().solution().set(_diag_save_in[i]->nodalDofIndex(), cached_val);
    }
  }
}

void
NodalBC::computeOffDiagJacobian(unsigned int jvar)
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

    // Cache the user's computeQpJacobian() value for later use.
    _fe_problem.assembly(0).cacheJacobianContribution(cached_row, cached_col, cached_val);
  }
}

Real
NodalBC::computeQpJacobian()
{
  return 1.;
}

Real
NodalBC::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.;
}
