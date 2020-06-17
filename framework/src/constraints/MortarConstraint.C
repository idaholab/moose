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
#include "MooseVariable.h"
#include "Assembly.h"

defineLegacyParams(MortarConstraint);

InputParameters
MortarConstraint::validParams()
{
  return MortarConstraintBase::validParams();
}

MortarConstraint::MortarConstraint(const InputParameters & parameters)
  : MortarConstraintBase(parameters),
    _lambda_dummy(),
    _lambda(_var ? _var->slnLower() : _lambda_dummy),
    _u_secondary(_secondary_var.sln()),
    _u_master(_master_var.slnNeighbor()),
    _grad_u_secondary(_secondary_var.gradSln()),
    _grad_u_master(_master_var.gradSlnNeighbor()),
    _phi(nullptr),
    _grad_phi(nullptr)
{
}

void
MortarConstraint::computeResidual(Moose::MortarType mortar_type)
{
  unsigned int test_space_size = 0;
  switch (mortar_type)
  {
    case Moose::MortarType::Slave:
      prepareVectorTag(_assembly, _secondary_var.number());
      test_space_size = _test_secondary.size();
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

void
MortarConstraint::computeJacobian(Moose::MortarType mortar_type)
{
  size_t test_space_size = 0;
  typedef Moose::ConstraintJacobianType JType;
  typedef Moose::MortarType MType;
  std::array<JType, 3> jacobian_types;

  switch (mortar_type)
  {
    case MType::Slave:
      test_space_size = _secondary_var.dofIndices().size();
      jacobian_types = {{JType::SlaveSlave, JType::SlaveMaster, JType::SlaveLower}};
      break;

    case MType::Master:
      test_space_size = _master_var.dofIndicesNeighbor().size();
      jacobian_types = {{JType::MasterSlave, JType::MasterMaster, JType::MasterLower}};
      break;

    case MType::Lower:
      test_space_size = _var ? _var->dofIndicesLower().size() : 0;
      jacobian_types = {{JType::LowerSlave, JType::LowerMaster, JType::LowerLower}};
      break;
  }

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
        if (ivar != _secondary_var.number())
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

    std::array<size_t, 3> shape_space_sizes{{jvariable.dofIndices().size(),
                                             jvariable.dofIndicesNeighbor().size(),
                                             jvariable.dofIndicesLower().size()}};
    std::array<const VariablePhiValue *, 3> phis;
    std::array<const VariablePhiGradient *, 3> grad_phis;
    std::array<const VectorVariablePhiValue *, 3> vector_phis;
    std::array<const VectorVariablePhiGradient *, 3> vector_grad_phis;
    if (jvariable.isVector())
    {
      const auto & temp_var = static_cast<MooseVariableFE<RealVectorValue> &>(jvariable);
      vector_phis = {{&temp_var.phiFace(), &temp_var.phiFaceNeighbor(), &temp_var.phiLower()}};
      vector_grad_phis = {
          {&temp_var.gradPhiFace(), &temp_var.gradPhiFaceNeighbor(), &temp_var.gradPhiLower()}};
    }
    else
    {
      const auto & temp_var = static_cast<MooseVariableFE<Real> &>(jvariable);
      phis = {{&temp_var.phiFace(), &temp_var.phiFaceNeighbor(), &temp_var.phiLower()}};
      grad_phis = {
          {&temp_var.gradPhiFace(), &temp_var.gradPhiFaceNeighbor(), &temp_var.gradPhiLower()}};
    }

    for (MooseIndex(3) type_index = 0; type_index < 3; ++type_index)
    {
      // If we don't have a master element, then we shouldn't be considering derivatives with
      // respect to master dofs. More practically speaking, the local K matrix will be improperly
      // sized whenever we don't have a master element because we won't be calling
      // FEProblemBase::reinitNeighborFaceRef from withing ComputeMortarFunctor::operator()
      if (type_index == 1 && !_has_master)
        continue;

      prepareMatrixTagLower(_assembly, ivar, jvar, jacobian_types[type_index]);

      /// Set the proper phis
      if (jvariable.isVector())
      {
        _vector_phi = vector_phis[type_index];
        _vector_grad_phi = vector_grad_phis[type_index];
      }
      else
      {
        _phi = phis[type_index];
        _grad_phi = grad_phis[type_index];
      }

      for (_i = 0; _i < test_space_size; _i++)
        for (_j = 0; _j < shape_space_sizes[type_index]; _j++)
          for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
            _local_ke(_i, _j) +=
                _JxW_msm[_qp] * _coord[_qp] * computeQpJacobian(jacobian_types[type_index], jvar);
      accumulateTaggedLocalMatrix();
    }
  }
}
