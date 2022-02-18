//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RankTwoBasedFailureCriteriaNOSPD.h"
#include "RankTwoScalarTools.h"

registerMooseObject("PeridynamicsApp", RankTwoBasedFailureCriteriaNOSPD);

InputParameters
RankTwoBasedFailureCriteriaNOSPD::validParams()
{
  InputParameters params = BondStatusBasePD::validParams();
  params.addClassDescription(
      "Class for rank two tensor based failure criteria in non-ordinary state-based model");

  params.addRequiredParam<MaterialPropertyName>("rank_two_tensor",
                                                "The rank two material tensor name");
  MooseEnum FailureCriterionTypes(
      "Axial MaxPrincipal TrescaStrain TrescaStress VonMisesStrain VonMisesStress");
  params.addRequiredParam<MooseEnum>("failure_criterion",
                                     FailureCriterionTypes,
                                     "Which stress based failure criterion to be used");

  return params;
}

RankTwoBasedFailureCriteriaNOSPD::RankTwoBasedFailureCriteriaNOSPD(
    const InputParameters & parameters)
  : BondStatusBasePD(parameters),
    _failure_criterion(getParam<MooseEnum>("failure_criterion")),
    _tensor(nullptr)
{
  if (hasMaterialProperty<RankTwoTensor>("rank_two_tensor"))
    _tensor = &getMaterialProperty<RankTwoTensor>("rank_two_tensor");
  else
    mooseError("Error in RankTwoBasedFailureCriteriaNOSPD! Required rank two tensor is not "
               "available for current peridynamics model!");

  if (isNodal())
    paramError("variable", "This AuxKernel only supports Elemental fields");
}

Real
RankTwoBasedFailureCriteriaNOSPD::computeFailureCriterionValue()
{
  Real val = 0.0;
  RankTwoTensor avg_tensor = 0.5 * ((*_tensor)[0] + (*_tensor)[1]);
  Point dirc;

  switch (_failure_criterion)
  {
    case 0:
      val = RankTwoScalarTools::axialStress(
          avg_tensor, *_current_elem->node_ptr(0), *_current_elem->node_ptr(1), dirc);
      break;
    case 1:
      val = RankTwoScalarTools::maxPrincipal(avg_tensor, dirc);
      break;
    case 2:
      val = 4.0 / 3.0 * RankTwoScalarTools::maxShear(avg_tensor);
      break;
    case 3:
      val = 2.0 * RankTwoScalarTools::maxShear(avg_tensor);
      break;
    case 4:
      val = RankTwoScalarTools::effectiveStrain(avg_tensor);
      break;
    case 5:
      val = RankTwoScalarTools::vonMisesStress(avg_tensor);
      break;
    default:
      mooseError("Unsupported rank two tensor-based failure criterion. Choose from: Axial "
                 "MaxPrincipal TrescaStrain TrescaStress VonMisesStrain VonMisesStress");
  }

  return val - _critical_val[0];
}
