//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarConstraint.h"

// MOOSE includes
#include "FEProblemBase.h"
#include "MooseVariable.h"

defineADBaseValidParams(
    MortarConstraint,
    MortarConstraintBase,
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

    params.addRequiredParam<NonlinearVariableName>("slave_variable",
                                                   "Primal variable on slave surface.");
    params.addParam<NonlinearVariableName>(
        "master_variable",
        "Primal variable on master surface. If this parameter is not provided then the master "
        "variable will be initialized to the slave variable");
    params.addParam<NonlinearVariableName>(
        "variable",
        "The name of the lagrange multiplier variable that this constraint is applied to. This "
        "parameter may not be supplied in the case of using penalty methods for example");
    params.addParam<bool>("compute_primal_residuals",
                          true,
                          "Whether to compute residuals for the primal variable.");
    params.addParam<bool>("compute_lm_residuals",
                          true,
                          "Whether to compute Lagrange Multiplier residuals");
    params.addParam<bool>(
        "periodic",
        false,
        "Whether this constraint is going to be used to enforce a periodic condition. This has the "
        "effect of changing the normals vector for projection from outward to inward facing"););

template <ComputeStage compute_stage>
MortarConstraint<compute_stage>::MortarConstraint(const InputParameters & parameters)
  : MortarConstraintBase(parameters),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _slave_id(_mesh.getBoundaryID(getParam<BoundaryName>("slave_boundary"))),
    _master_id(_mesh.getBoundaryID(getParam<BoundaryName>("master_boundary"))),
    _slave_subdomain_id(_mesh.getSubdomainID(getParam<SubdomainName>("slave_subdomain"))),
    _master_subdomain_id(_mesh.getSubdomainID(getParam<SubdomainName>("master_subdomain"))),
    _var(isParamValid("variable")
             ? &_subproblem.getStandardVariable(_tid, parameters.getMooseType("variable"))
             : nullptr),
    _slave_var(_subproblem.getStandardVariable(_tid, parameters.getMooseType("slave_variable"))),
    _master_var(
        isParamValid("master_variable")
            ? _subproblem.getStandardVariable(_tid, parameters.getMooseType("master_variable"))
            : _slave_var),

    _compute_primal_residuals(getParam<bool>("compute_primal_residuals")),
    _compute_lm_residuals(!_var ? false : getParam<bool>("compute_lm_residuals")),
    _test_dummy(),
    _lambda_dummy(),
    _normals(_assembly.normals()),
    _JxW_msm(_assembly.jxWMortar()),
    _coord(_assembly.coordTransformation()),
    _qrule_msm(_assembly.qRuleMortar()),
    _test(_var ? _var->phiLower() : _test_dummy),
    _test_slave(_slave_var.phiFace()),
    _test_master(_master_var.phiFaceNeighbor()),
    _grad_test_slave(_slave_var.gradPhiFace()),
    _grad_test_master(_master_var.gradPhiFaceNeighbor()),
    _phys_points_slave(_assembly.qPointsFace()),
    _phys_points_master(_assembly.qPointsFaceNeighbor()),
    _lambda(_var ? _var->adSlnLower<compute_stage>() : _lambda_dummy),
    _u_slave(_slave_var.adSln<compute_stage>()),
    _u_master(_master_var.adSlnNeighbor<compute_stage>()),
    _grad_u_slave(_slave_var.adGradSln<compute_stage>()),
    _grad_u_master(_master_var.adGradSlnNeighbor<compute_stage>())
{
  _fe_problem.createMortarInterface(std::make_pair(_master_id, _slave_id),
                                    std::make_pair(_master_subdomain_id, _slave_subdomain_id),
                                    getParam<bool>("use_displaced_mesh"),
                                    getParam<bool>("periodic"));
}

template <ComputeStage compute_stage>
void
MortarConstraint<compute_stage>::computeResidual(bool has_master)
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

template <>
void
MortarConstraint<JACOBIAN>::computeResidual(bool /*has_master*/)
{
}

template <ComputeStage compute_stage>
void
MortarConstraint<compute_stage>::computeResidual(Moose::MortarType mortar_type)
{
  unsigned int test_space_size = 0;
  switch (mortar_type)
  {
    case Moose::MortarType::Slave:
      prepareVectorTag(_assembly, _slave_var.number());
      test_space_size = _test_slave.size();
      break;

    case Moose::MortarType::Master:
      prepareVectorTagNeighbor(_assembly, _master_var.number());
      test_space_size = _test_master.size();
      break;

    case Moose::MortarType::Lower:
      mooseAssert(_var, "LM variable is null");
      prepareVectorTagLower(_assembly, _var->number());
      test_space_size = _test.size();
      break;
  }

  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
    for (_i = 0; _i < test_space_size; _i++)
      _local_re(_i) += _JxW_msm[_qp] * _coord[_qp] * computeQpResidual(mortar_type);

  accumulateTaggedLocalResidual();
}

template <>
void MortarConstraint<JACOBIAN>::computeResidual(Moose::MortarType /*mortar_type*/)
{
}

template <ComputeStage compute_stage>
void
MortarConstraint<compute_stage>::computeJacobian(bool has_master)
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

template <>
void
MortarConstraint<RESIDUAL>::computeJacobian(bool /*has_master*/)
{
}

template <ComputeStage compute_stage>
void
MortarConstraint<compute_stage>::computeJacobian(Moose::MortarType mortar_type)
{
  std::vector<DualReal> residuals;
  size_t test_space_size = 0;
  typedef Moose::ConstraintJacobianType JType;
  typedef Moose::MortarType MType;
  std::vector<JType> jacobian_types;

  switch (mortar_type)
  {
    case MType::Slave:
      test_space_size = _slave_var.dofIndices().size();
      jacobian_types = {JType::SlaveSlave, JType::SlaveMaster, JType::SlaveLower};
      break;

    case MType::Master:
      test_space_size = _master_var.dofIndicesNeighbor().size();
      jacobian_types = {JType::MasterSlave, JType::MasterMaster, JType::MasterLower};
      break;

    case MType::Lower:
      test_space_size = _var ? _var->dofIndicesLower().size() : 0;
      jacobian_types = {JType::LowerSlave, JType::LowerMaster, JType::LowerLower};
      break;
  }

  residuals.resize(test_space_size, 0);
  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
    for (_i = 0; _i < test_space_size; _i++)
      residuals[_i] += _JxW_msm[_qp] * _coord[_qp] * computeQpResidual(mortar_type);

  std::vector<std::pair<MooseVariableFEBase *, MooseVariableFEBase *>> & ce =
      _assembly.couplingEntries();
  for (const auto & it : ce)
  {
    MooseVariableFEBase & ivariable = *(it.first);
    MooseVariableFEBase & jvariable = *(it.second);

    unsigned int ivar = ivariable.number();
    unsigned int jvar = jvariable.number();

    switch (mortar_type)
    {
      case MType::Slave:
        if (ivar != _slave_var.number())
          continue;
        break;

      case MType::Master:
        if (ivar != _master_var.number())
          continue;
        break;

      case MType::Lower:
        if (!_var || _var->number() != ivar)
          continue;
        break;
    }

    // Derivatives are offset by the variable number
    std::vector<size_t> ad_offsets{jvar * _sys.getMaxVarNDofsPerElem(),
                                   jvar * _sys.getMaxVarNDofsPerElem() +
                                       (_sys.system().n_vars() * _sys.getMaxVarNDofsPerElem()),
                                   2 * _sys.system().n_vars() * _sys.getMaxVarNDofsPerElem() +
                                       jvar * _sys.getMaxVarNDofsPerElem()};
    std::vector<size_t> shape_space_sizes{jvariable.dofIndices().size(),
                                          jvariable.dofIndicesNeighbor().size(),
                                          jvariable.dofIndicesLower().size()};

    for (MooseIndex(3) type_index = 0; type_index < 3; ++type_index)
    {
      prepareMatrixTagLower(_assembly, ivar, jvar, jacobian_types[type_index]);
      for (_i = 0; _i < test_space_size; _i++)
        for (_j = 0; _j < shape_space_sizes[type_index]; _j++)
          _local_ke(_i, _j) += residuals[_i].derivatives()[ad_offsets[type_index] + _j];
      accumulateTaggedLocalMatrix();
    }
  }
}

template <>
void MortarConstraint<RESIDUAL>::computeJacobian(Moose::MortarType /*mortar_type*/)
{
}

adBaseClass(MortarConstraint);
