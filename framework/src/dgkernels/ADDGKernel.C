//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADDGKernel.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "NonlinearSystemBase.h"
#include "ADUtils.h"

// libmesh includes
#include "libmesh/threads.h"

InputParameters
ADDGKernel::validParams()
{
  InputParameters params = DGKernelBase::validParams();
  params.addClassDescription(
      "Base class for all DG kernels making use of automatic differentiation");
  return params;
}

ADDGKernel::ADDGKernel(const InputParameters & parameters)
  : DGKernelBase(parameters),
    NeighborMooseVariableInterface(
        this, false, Moose::VarKindType::VAR_NONLINEAR, Moose::VarFieldType::VAR_FIELD_STANDARD),
    _var(*mooseVariable()),
    _phi(_assembly.phiFace(_var)),
    _grad_phi(_assembly.gradPhiFace(_var)),

    _test(_var.phiFace()),
    _grad_test(_var.gradPhiFace()),

    _phi_neighbor(_assembly.phiFaceNeighbor(_var)),
    _grad_phi_neighbor(_assembly.gradPhiFaceNeighbor(_var)),

    _test_neighbor(_var.phiFaceNeighbor()),
    _grad_test_neighbor(_var.gradPhiFaceNeighbor()),

    _u(_var.adSln()),
    _grad_u(_var.adGradSln()),
    _u_neighbor(_var.adSlnNeighbor()),
    _grad_u_neighbor(_var.adGradSlnNeighbor())
{
  _subproblem.haveADObjects(true);

  addMooseVariableDependency(mooseVariable());

  _save_in.resize(_save_in_strings.size());
  _diag_save_in.resize(_diag_save_in_strings.size());

  for (unsigned int i = 0; i < _save_in_strings.size(); i++)
  {
    MooseVariableFEBase * var = &_subproblem.getVariable(_tid,
                                                         _save_in_strings[i],
                                                         Moose::VarKindType::VAR_AUXILIARY,
                                                         Moose::VarFieldType::VAR_FIELD_STANDARD);

    if (_sys.hasVariable(_save_in_strings[i]))
      mooseError("Trying to use solution variable " + _save_in_strings[i] +
                 " as a save_in variable in " + name());

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
    MooseVariableFEBase * var = &_subproblem.getVariable(_tid,
                                                         _diag_save_in_strings[i],
                                                         Moose::VarKindType::VAR_NONLINEAR,
                                                         Moose::VarFieldType::VAR_FIELD_STANDARD);

    if (_sys.hasVariable(_diag_save_in_strings[i]))
      mooseError("Trying to use solution variable " + _diag_save_in_strings[i] +
                 " as a diag_save_in variable in " + name());

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
ADDGKernel::computeElemNeighResidual(Moose::DGResidualType type)
{
  bool is_elem;
  if (type == Moose::Element)
    is_elem = true;
  else
    is_elem = false;

  const VariableTestValue & test_space = is_elem ? _test : _test_neighbor;

  if (is_elem)
    prepareVectorTag(_assembly, _var.number());
  else
    prepareVectorTagNeighbor(_assembly, _var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      _local_re(_i) += raw_value(_JxW[_qp] * _coord[_qp] * computeQpResidual(type));

  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(_resid_vars_mutex);
    for (const auto & var : _save_in)
    {
      const std::vector<dof_id_type> & dof_indices =
          is_elem ? var->dofIndices() : var->dofIndicesNeighbor();
      var->sys().solution().add_vector(_local_re, dof_indices);
    }
  }
}

void
ADDGKernel::computeJacobian()
{
  // AD only needs to do one computation for one variable because it does the derivatives all at
  // once
  if (!excludeBoundary())
  {
    computeElemNeighJacobian(Moose::ElementElement);
    computeElemNeighJacobian(Moose::NeighborNeighbor);
  }
}

void
ADDGKernel::computeElemNeighJacobian(Moose::DGJacobianType type)
{
  mooseAssert(type == Moose::ElementElement || type == Moose::NeighborNeighbor,
              "With AD you should need one call per side");

  const VariableTestValue & test_space = type == Moose::ElementElement ? _test : _test_neighbor;

  std::vector<ADReal> residuals(test_space.size(), 0);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      residuals[_i] +=
          _JxW[_qp] * _coord[_qp] *
          computeQpResidual(type == Moose::ElementElement ? Moose::Element : Moose::Neighbor);

  auto local_functor = [&](const std::vector<ADReal> & input_residuals,
                           const std::vector<dof_id_type> &,
                           const std::set<TagID> &)
  {
    auto compute_jacobian_type = [&](const Moose::DGJacobianType nested_type)
    {
      const VariableTestValue & loc_phi =
          (nested_type == Moose::ElementElement || nested_type == Moose::NeighborElement)
              ? _phi
              : _phi_neighbor;

      prepareMatrixTagNeighbor(_assembly, _var.number(), _var.number(), nested_type);

      auto ad_offset = Moose::adOffset(
          _var.number(), _sys.getMaxVarNDofsPerElem(), nested_type, _sys.system().n_vars());

      for (_i = 0; _i < test_space.size(); _i++)
        for (_j = 0; _j < loc_phi.size(); _j++)
          _local_ke(_i, _j) += input_residuals[_i].derivatives()[ad_offset + _j];

      accumulateTaggedLocalMatrix();
    };

    if (type == Moose::ElementElement)
    {
      compute_jacobian_type(Moose::ElementElement);
      compute_jacobian_type(Moose::ElementNeighbor);
    }
    else
    {
      compute_jacobian_type(Moose::NeighborElement);
      compute_jacobian_type(Moose::NeighborNeighbor);
    }
  };

  _assembly.processJacobian(residuals,
                            type == Moose::ElementElement ? _var.dofIndices()
                                                          : _var.dofIndicesNeighbor(),
                            _matrix_tags,
                            _var.scalingFactor(),
                            local_functor);

  if (_has_diag_save_in)
  {
    unsigned int rows = _local_ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i = 0; i < rows; i++)
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(_jacoby_vars_mutex);
    for (const auto & var : _diag_save_in)
    {
      if (type == Moose::ElementElement)
        var->sys().solution().add_vector(diag, var->dofIndices());
      else
        var->sys().solution().add_vector(diag, var->dofIndicesNeighbor());
    }
  }
}

void
ADDGKernel::computeOffDiagJacobian(const unsigned int jvar_num)
{
  // AD only needs to do one computation for one variable because it does the derivatives all at
  // once
  if (!excludeBoundary() && jvar_num == _var.number())
  {
    const auto & jvar = getVariable(jvar_num);
    computeOffDiagElemNeighJacobian(Moose::ElementElement, jvar);
    computeOffDiagElemNeighJacobian(Moose::NeighborNeighbor, jvar);
  }
}

void
ADDGKernel::computeOffDiagElemNeighJacobian(Moose::DGJacobianType type, const MooseVariableFEBase &)
{
  mooseAssert(type == Moose::ElementElement || type == Moose::NeighborNeighbor,
              "With AD you should need one call per side");

  const VariableTestValue & test_space = type == Moose::ElementElement ? _test : _test_neighbor;

  std::vector<ADReal> residuals(test_space.size(), 0);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      residuals[_i] +=
          _JxW[_qp] * _coord[_qp] *
          computeQpResidual(type == Moose::ElementElement ? Moose::Element : Moose::Neighbor);

  auto local_functor = [&](const std::vector<ADReal> & input_residuals,
                           const std::vector<dof_id_type> &,
                           const std::set<TagID> &)
  {
    auto & ce = _assembly.couplingEntries();
    for (const auto & it : ce)
    {
      MooseVariableFEBase & ivariable = *(it.first);
      MooseVariableFEBase & jvariable = *(it.second);

      unsigned int ivar = ivariable.number();
      unsigned int jvar = jvariable.number();

      if (ivar != _var.number())
        continue;

      auto compute_jacobian_type = [&](const Moose::DGJacobianType nested_type)
      {
        const VariableTestValue & loc_phi =
            (nested_type == Moose::ElementElement || nested_type == Moose::NeighborElement)
                ? _phi
                : _phi_neighbor;
        const auto jsub =
            (nested_type == Moose::ElementElement || nested_type == Moose::NeighborElement)
                ? _current_elem->subdomain_id()
                : _neighbor_elem->subdomain_id();

        if (!jvariable.hasBlocks(jsub))
          return;

        prepareMatrixTagNeighbor(_assembly, _var.number(), jvar, nested_type);

        auto ad_offset = Moose::adOffset(
            jvar, _sys.getMaxVarNDofsPerElem(), nested_type, _sys.system().n_vars());

        for (_i = 0; _i < test_space.size(); _i++)
          for (_j = 0; _j < loc_phi.size(); _j++)
            _local_ke(_i, _j) += input_residuals[_i].derivatives()[ad_offset + _j];

        accumulateTaggedLocalMatrix();
      };

      if (type == Moose::ElementElement)
      {
        compute_jacobian_type(Moose::ElementElement);
        compute_jacobian_type(Moose::ElementNeighbor);
      }
      else
      {
        compute_jacobian_type(Moose::NeighborElement);
        compute_jacobian_type(Moose::NeighborNeighbor);
      }
    }
  };

  _assembly.processJacobian(residuals,
                            type == Moose::ElementElement ? _var.dofIndices()
                                                          : _var.dofIndicesNeighbor(),
                            _matrix_tags,
                            _var.scalingFactor(),
                            local_functor);
}
