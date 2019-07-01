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

template <>
InputParameters
validParams<MortarConstraintBase>()
{
  InputParameters params = validParams<Constraint>();
  params.addRequiredParam<BoundaryName>("master_boundary",
                                        "The name of the master boundary sideset.");
  params.addRequiredParam<BoundaryName>("slave_boundary",
                                        "The name of the slave boundary sideset.");
  params.addRequiredParam<SubdomainName>("master_subdomain", "The name of the master subdomain.");
  params.addRequiredParam<SubdomainName>("slave_subdomain", "The name of the slave subdomain.");
  params.addRelationshipManager(
      "AugmentSparsityOnInterface",
      Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC,
      [](const InputParameters & obj_params, InputParameters & rm_params) {
        rm_params.set<bool>("use_displaced_mesh") = obj_params.get<bool>("use_displaced_mesh");
        rm_params.set<BoundaryName>("slave_boundary") =
            obj_params.get<BoundaryName>("slave_boundary");
        rm_params.set<BoundaryName>("master_boundary") =
            obj_params.get<BoundaryName>("master_boundary");
        rm_params.set<SubdomainName>("slave_subdomain") =
            obj_params.get<SubdomainName>("slave_subdomain");
        rm_params.set<SubdomainName>("master_subdomain") =
            obj_params.get<SubdomainName>("master_subdomain");
      });

  params.addParam<VariableName>("slave_variable", "Primal variable on slave surface.");
  params.addParam<VariableName>(
      "master_variable",
      "Primal variable on master surface. If this parameter is not provided then the master "
      "variable will be initialized to the slave variable");
  params.addParam<NonlinearVariableName>(
      "variable",
      "The name of the lagrange multiplier variable that this constraint is applied to. This "
      "parameter may not be supplied in the case of using penalty methods for example");
  params.addParam<bool>(
      "compute_primal_residuals", true, "Whether to compute residuals for the primal variable.");
  params.addParam<bool>(
      "compute_lm_residuals", true, "Whether to compute Lagrange Multiplier residuals");
  params.addParam<bool>(
      "periodic",
      false,
      "Whether this constraint is going to be used to enforce a periodic condition. This has the "
      "effect of changing the normals vector for projection from outward to inward facing");
  return params;
}

MortarConstraintBase::MortarConstraintBase(const InputParameters & parameters)
  : Constraint(parameters),
    CoupleableMooseVariableDependencyIntermediateInterface(this, true),
    MooseVariableInterface<Real>(this,
                                 true,
                                 "variable",
                                 Moose::VarKindType::VAR_NONLINEAR,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _slave_id(_mesh.getBoundaryID(getParam<BoundaryName>("slave_boundary"))),
    _master_id(_mesh.getBoundaryID(getParam<BoundaryName>("master_boundary"))),
    _slave_subdomain_id(_mesh.getSubdomainID(getParam<SubdomainName>("slave_subdomain"))),
    _master_subdomain_id(_mesh.getSubdomainID(getParam<SubdomainName>("master_subdomain"))),
    _var(isParamValid("variable")
             ? &_subproblem.getStandardVariable(_tid, parameters.getMooseType("variable"))
             : nullptr),
    _slave_var(
        isParamValid("slave_variable")
            ? _subproblem.getStandardVariable(_tid, parameters.getMooseType("slave_variable"))
            : _subproblem.getStandardVariable(_tid, parameters.getMooseType("master_variable"))),
    _master_var(
        isParamValid("master_variable")
            ? _subproblem.getStandardVariable(_tid, parameters.getMooseType("master_variable"))
            : _slave_var),

    _compute_primal_residuals(getParam<bool>("compute_primal_residuals")),
    _compute_lm_residuals(!_var ? false : getParam<bool>("compute_lm_residuals")),
    _test_dummy(),
    _normals(_assembly.normals()),
    _tangents(_assembly.tangents()),
    _JxW_msm(_assembly.jxWMortar()),
    _coord(_assembly.coordTransformation()),
    _qrule_msm(_assembly.qRuleMortar()),
    _test(_var ? _var->phiLower() : _test_dummy),
    _test_slave(_slave_var.phiFace()),
    _test_master(_master_var.phiFaceNeighbor()),
    _grad_test_slave(_slave_var.gradPhiFace()),
    _grad_test_master(_master_var.gradPhiFaceNeighbor()),
    _phys_points_slave(_assembly.qPointsFace()),
    _phys_points_master(_assembly.qPointsFaceNeighbor())
{
  _fe_problem.createMortarInterface(std::make_pair(_master_id, _slave_id),
                                    std::make_pair(_master_subdomain_id, _slave_subdomain_id),
                                    getParam<bool>("use_displaced_mesh"),
                                    getParam<bool>("periodic"));
}

void
MortarConstraintBase::computeResidual(bool has_master)
{
  _has_master = has_master;

  if (_compute_primal_residuals)
  {
    // Compute the residual for the slave interior primal dofs
    computeResidual(Moose::MortarType::Slave);

    // Compute the residual for the master interior primal dofs
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
    // Compute the jacobian for the slave interior primal dofs
    computeJacobian(Moose::MortarType::Slave);

    // Compute the jacobian for the master interior primal dofs
    computeJacobian(Moose::MortarType::Master);
  }

  if (_compute_lm_residuals)
    // Compute the jacobian for the lower dimensional LM dofs (if we even have an LM variable)
    computeJacobian(Moose::MortarType::Lower);
}
