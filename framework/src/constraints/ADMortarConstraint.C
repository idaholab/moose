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
#include "ADUtils.h"

InputParameters
ADMortarConstraint::validParams()
{
  InputParameters params = MortarConstraintBase::validParams();
  return params;
}

ADMortarConstraint::ADMortarConstraint(const InputParameters & parameters)
  : MortarConstraintBase(parameters),
    _lambda_dummy(),
    _lambda(_var ? _var->adSlnLower() : _lambda_dummy),
    _u_secondary(_secondary_var.adSln()),
    _u_master(_master_var.adSlnNeighbor()),
    _grad_u_secondary(_secondary_var.adGradSln()),
    _grad_u_master(_master_var.adGradSlnNeighbor())
{
  _subproblem.haveADObjects(true);
}

void
ADMortarConstraint::computeResidual(Moose::MortarType mortar_type)
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
      _local_re(_i) += raw_value(_JxW_msm[_qp] * _coord[_qp] * computeQpResidual(mortar_type));

  accumulateTaggedLocalResidual();
}

void
ADMortarConstraint::computeJacobian(Moose::MortarType mortar_type)
{
  std::vector<DualReal> residuals;
  size_t test_space_size = 0;
  typedef Moose::ConstraintJacobianType JType;
  typedef Moose::MortarType MType;
  std::vector<JType> jacobian_types;

  switch (mortar_type)
  {
    case MType::Slave:
      test_space_size = _secondary_var.dofIndices().size();
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

    // Derivatives are offset by the variable number
    std::vector<size_t> ad_offsets{
        Moose::adOffset(jvar, _sys.getMaxVarNDofsPerElem(), Moose::ElementType::Element),
        Moose::adOffset(jvar,
                        _sys.getMaxVarNDofsPerElem(),
                        Moose::ElementType::Neighbor,
                        _sys.system().n_vars()),
        Moose::adOffset(
            jvar, _sys.getMaxVarNDofsPerElem(), Moose::ElementType::Lower, _sys.system().n_vars())};
    std::vector<size_t> shape_space_sizes{jvariable.dofIndices().size(),
                                          jvariable.dofIndicesNeighbor().size(),
                                          jvariable.dofIndicesLower().size()};

    for (MooseIndex(3) type_index = 0; type_index < 3; ++type_index)
    {
      // If we don't have a master element, then we shouldn't be considering derivatives with
      // respect to master dofs. More practically speaking, the local K matrix will be improperly
      // sized whenever we don't have a master element because we won't be calling
      // FEProblemBase::reinitNeighborFaceRef from withing ComputeMortarFunctor::operator()
      if (type_index == 1 && !_has_master)
        continue;

      prepareMatrixTagLower(_assembly, ivar, jvar, jacobian_types[type_index]);
      for (_i = 0; _i < test_space_size; _i++)
        for (_j = 0; _j < shape_space_sizes[type_index]; _j++)
          _local_ke(_i, _j) += residuals[_i].derivatives()[ad_offsets[type_index] + _j];
      accumulateTaggedLocalMatrix();
    }
  }
}
