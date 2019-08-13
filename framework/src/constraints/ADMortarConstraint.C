//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMortarConstraint.h"

// MOOSE includes
#include "MooseVariable.h"
#include "Assembly.h"
#include "SystemBase.h"

defineADBaseValidParams(ADMortarConstraint, MortarConstraintBase, );

template <ComputeStage compute_stage>
ADMortarConstraint<compute_stage>::ADMortarConstraint(const InputParameters & parameters)
  : MortarConstraintBase(parameters),
    _lambda_dummy(),
    _lambda(_var ? _var->adSlnLower<compute_stage>() : _lambda_dummy),
    _u_slave(_slave_var.adSln<compute_stage>()),
    _u_master(_master_var.adSlnNeighbor<compute_stage>()),
    _grad_u_slave(_slave_var.adGradSln<compute_stage>()),
    _grad_u_master(_master_var.adGradSlnNeighbor<compute_stage>())
{
}

template <ComputeStage compute_stage>
void
ADMortarConstraint<compute_stage>::computeResidual(bool has_master)
{
  MortarConstraintBase::computeResidual(has_master);
}

template <>
void
ADMortarConstraint<JACOBIAN>::computeResidual(bool /*has_master*/)
{
}

template <ComputeStage compute_stage>
void
ADMortarConstraint<compute_stage>::computeResidual(Moose::MortarType mortar_type)
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
void ADMortarConstraint<JACOBIAN>::computeResidual(Moose::MortarType /*mortar_type*/)
{
}

template <ComputeStage compute_stage>
void
ADMortarConstraint<compute_stage>::computeJacobian(bool has_master)
{
  MortarConstraintBase::computeJacobian(has_master);
}

template <>
void
ADMortarConstraint<RESIDUAL>::computeJacobian(bool /*has_master*/)
{
}

template <ComputeStage compute_stage>
void
ADMortarConstraint<compute_stage>::computeJacobian(Moose::MortarType mortar_type)
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

  auto & ce = _assembly.couplingEntries();
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
void ADMortarConstraint<RESIDUAL>::computeJacobian(Moose::MortarType /*mortar_type*/)
{
}

adBaseClass(ADMortarConstraint);
