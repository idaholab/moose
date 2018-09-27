//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BondStatusPD.h"
#include "MeshBasePD.h"

registerMooseObject("PeridynamicsApp", BondStatusPD);

template <>
InputParameters
validParams<BondStatusPD>()
{
  InputParameters params = validParams<AuxKernelBasePD>();
  params.addClassDescription("Class for updating the bond status based on different failure "
                             "criteria: critical stretch and "
                             "maximum principal stress");
  MooseEnum FailureCriteriaType("CriticalStretch MaximumTensileStress", "CriticalStretch");
  params.addParam<MooseEnum>(
      "failure_criterion", FailureCriteriaType, "Which failure criterion to be used");
  params.addRequiredCoupledVar("critical_variable", "Name of critical AuxVariable");
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;

  return params;
}

BondStatusPD::BondStatusPD(const InputParameters & parameters)
  : AuxKernelBasePD(parameters),
    _failure_criterion(getParam<MooseEnum>("failure_criterion").getEnum<FailureCriterion>()),
    _bond_status_var(_subproblem.getVariable(_tid, "bond_status")),
    _critical_val(coupledValue("critical_variable")),
    _mechanical_stretch(getMaterialProperty<Real>("mechanical_stretch")),
    _stress(NULL)
{
  switch (_failure_criterion)
  {
    case FailureCriterion::CriticalStretch:
      break;

    case FailureCriterion::MaximumTensileStress:
    {
      if (hasMaterialProperty<RankTwoTensor>("stress"))
        _stress = &getMaterialProperty<RankTwoTensor>("stress");
      else
        mooseError("Material property stress is not available for current model!");

      break;
    }

    default:
      paramError("failure_criterion", "Unsupported PD failure criterion. Choose from: CriticalStretch and "
                 "MaximumPrincipalStress");
  }
}

Real
BondStatusPD::computeValue()
{
  Real val = 0.0;

  switch (_failure_criterion)
  {
    case FailureCriterion::CriticalStretch:
      val = _mechanical_stretch[0];
      break;

    case FailureCriterion::MaximumTensileStress:
    {
      RankTwoTensor avg_stress = 0.5 * ((*_stress)[0] + (*_stress)[1]);
      std::vector<Real> eigvals(LIBMESH_DIM, 0.0);

      if (_bond_status_var.getElementalValue(_current_elem) > 0.5)
        avg_stress.symmetricEigenvalues(eigvals);

      val = eigvals[LIBMESH_DIM - 1];
      break;
    }

    default:
      paramError("failure_criterion", "Unsupported PD failure criterion. Choose from: CriticalStretch and "
                 "MaximumPrincipalStress");
  }

  if (_bond_status_var.getElementalValue(_current_elem) > 0.5 && val < _critical_val[0])
    return 1.0; // unbroken and does not meet the failure criterion, bond is still unbroken
  else
    return 0.0; // meet the failure criterion, bond is taken as broken
}
