//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TensileStressUpdate_PartiallyImplemented.h"

registerMooseObject("SolidMechanicsTestApp", TensileStressUpdate_PartiallyImplemented);

InputParameters
TensileStressUpdate_PartiallyImplemented::validParams()
{
  InputParameters params = TensileStressUpdate::validParams();
  params.addClassDescription(
      "Physically, this is just TensileStressUpdate, but the code uses "
      "MultiParameterPlasticityStressUpdate::consistentTangentOperatorV instead of the optimised "
      "algorithm found in TensileStressUpdate.  The purpose is to show that the "
      "MultiParameterPlasticityStressUpdate algorithm is correct, although slow.");
  return params;
}

TensileStressUpdate_PartiallyImplemented::TensileStressUpdate_PartiallyImplemented(
    const InputParameters & parameters)
  : TensileStressUpdate(parameters)
{
}

void
TensileStressUpdate_PartiallyImplemented::consistentTangentOperatorV(
    const RankTwoTensor & stress_trial,
    const std::vector<Real> & trial_stress_params,
    const RankTwoTensor & stress,
    const std::vector<Real> & stress_params,
    Real gaE,
    const yieldAndFlow & smoothed_q,
    const RankFourTensor & elasticity_tensor,
    bool compute_full_tangent_operator,
    const std::vector<std::vector<Real>> & dvar_dtrial,
    RankFourTensor & cto)
{
  MultiParameterPlasticityStressUpdate::consistentTangentOperatorV(stress_trial,
                                                                   trial_stress_params,
                                                                   stress,
                                                                   stress_params,
                                                                   gaE,
                                                                   smoothed_q,
                                                                   elasticity_tensor,
                                                                   compute_full_tangent_operator,
                                                                   dvar_dtrial,
                                                                   cto);
}
