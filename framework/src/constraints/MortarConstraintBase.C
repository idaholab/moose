//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarConstraintBase.h"
#include "FEProblemBase.h"
#include "Assembly.h"
#include "MooseVariableFE.h"

defineLegacyParams(MortarConstraintBase);

InputParameters
MortarConstraintBase::validParams()
{
  InputParameters params = Constraint::validParams();
  params += MortarInterface::validParams();
  params += TwoMaterialPropertyInterface::validParams();

  params.addRelationshipManager(
      "AugmentSparsityOnInterface",
      Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC,
      [](const InputParameters & obj_params, InputParameters & rm_params) {
        rm_params.set<bool>("use_displaced_mesh") = obj_params.get<bool>("use_displaced_mesh");
        rm_params.set<BoundaryName>("secondary_boundary") =
            obj_params.get<BoundaryName>("secondary_boundary");
        rm_params.set<BoundaryName>("master_boundary") =
            obj_params.get<BoundaryName>("master_boundary");
        rm_params.set<SubdomainName>("secondary_subdomain") =
            obj_params.get<SubdomainName>("secondary_subdomain");
        rm_params.set<SubdomainName>("master_subdomain") =
            obj_params.get<SubdomainName>("master_subdomain");
      });

  params.addParam<VariableName>("secondary_variable", "Primal variable on secondary surface.");
  params.addParam<VariableName>(
      "master_variable",
      "Primal variable on master surface. If this parameter is not provided then the master "
      "variable will be initialized to the secondary variable");
  params.addParam<NonlinearVariableName>(
      "variable",
      "The name of the lagrange multiplier variable that this constraint is applied to. This "
      "parameter may not be supplied in the case of using penalty methods for example");
  params.addParam<bool>(
      "compute_primal_residuals", true, "Whether to compute residuals for the primal variable.");
  params.addParam<bool>(
      "compute_lm_residuals", true, "Whether to compute Lagrange Multiplier residuals");
  return params;
}

MortarConstraintBase::MortarConstraintBase(const InputParameters & parameters)
  : Constraint(parameters),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, false, false),
    MortarInterface(this),
    TwoMaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, getBoundaryIDs()),
    MooseVariableInterface<Real>(this,
                                 true,
                                 "variable",
                                 Moose::VarKindType::VAR_NONLINEAR,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _var(isParamValid("variable")
             ? &_subproblem.getStandardVariable(_tid, parameters.getMooseType("variable"))
             : nullptr),
    _secondary_var(
        isParamValid("secondary_variable")
            ? _subproblem.getStandardVariable(_tid, parameters.getMooseType("secondary_variable"))
            : _subproblem.getStandardVariable(_tid, parameters.getMooseType("master_variable"))),
    _master_var(
        isParamValid("master_variable")
            ? _subproblem.getStandardVariable(_tid, parameters.getMooseType("master_variable"))
            : _secondary_var),

    _compute_primal_residuals(getParam<bool>("compute_primal_residuals")),
    _compute_lm_residuals(!_var ? false : getParam<bool>("compute_lm_residuals")),
    _test_dummy(),
    _use_dual(_var ? _var->useDual() : false),
    _normals(_assembly.normals()),
    _tangents(_assembly.tangents()),
    _JxW_msm(_assembly.jxWMortar()),
    _coord(_assembly.mortarCoordTransformation()),
    _qrule_msm(_assembly.qRuleMortar()),
    _test(_var ? _var->phiLower() : _test_dummy),
    _test_secondary(_secondary_var.phiFace()),
    _test_master(_master_var.phiFaceNeighbor()),
    _grad_test_secondary(_secondary_var.gradPhiFace()),
    _grad_test_master(_master_var.gradPhiFaceNeighbor()),
    _phys_points_secondary(_assembly.qPointsFace()),
    _phys_points_master(_assembly.qPointsFaceNeighbor())
{
}

void
MortarConstraintBase::computeResidual(bool has_master)
{
  // Set this member for potential use by derived classes
  _has_master = has_master;

  if (_compute_primal_residuals)
  {
    // Compute the residual for the secondary interior primal dofs
    computeResidual(Moose::MortarType::Slave);

    // Compute the residual for the master interior primal dofs. If we don't have a master element,
    // then we don't have any master dofs
    if (_has_master)
      computeResidual(Moose::MortarType::Master);
  }

  if (_compute_lm_residuals)
    // Compute the residual for the lower dimensional LM dofs (if we even have an LM variable)
    computeResidual(Moose::MortarType::Lower);
}

void
MortarConstraintBase::computeJacobian(bool has_master)
{
  _has_master = has_master;

  if (_compute_primal_residuals)
  {
    // Compute the jacobian for the secondary interior primal dofs
    computeJacobian(Moose::MortarType::Slave);

    // Compute the jacobian for the master interior primal dofs. If we don't have a master element,
    // then we don't have any master dofs
    if (_has_master)
      computeJacobian(Moose::MortarType::Master);
  }

  if (_compute_lm_residuals)
    // Compute the jacobian for the lower dimensional LM dofs (if we even have an LM variable)
    computeJacobian(Moose::MortarType::Lower);
}
