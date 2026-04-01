//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CappedWeakPlaneStressUpdate_PartiallyImplemented.h"

#include "libmesh/utility.h"

registerMooseObject("SolidMechanicsTestApp", CappedWeakPlaneStressUpdate_PartiallyImplemented);

InputParameters
CappedWeakPlaneStressUpdate_PartiallyImplemented::validParams()
{
  InputParameters params = CappedWeakPlaneStressUpdate::validParams();
  params.addClassDescription(
      "Physically, this is just CappedWeakPlaneStressUpdate, but the code uses "
      "TwoParameterPlasticityStressUpdate::consistentTangentOperator instead of the optimised one "
      "in CappedWeakPlaneStressUpdate.  The purpose is to show that the "
      "TwoParameterPlasticityStressUpdate algorithm is correct, although slow.");
  return params;
}

CappedWeakPlaneStressUpdate_PartiallyImplemented::CappedWeakPlaneStressUpdate_PartiallyImplemented(
    const InputParameters & parameters)
  : CappedWeakPlaneStressUpdate(parameters)
{
}

void
CappedWeakPlaneStressUpdate_PartiallyImplemented::consistentTangentOperator(
    const RankTwoTensor & stress_trial,
    Real p_trial,
    Real q_trial,
    const RankTwoTensor & stress,
    Real p,
    Real q,
    Real gaE,
    const yieldAndFlow & smoothed_q,
    const RankFourTensor & Eijkl,
    bool compute_full_tangent_operator,
    RankFourTensor & cto) const
{
  TwoParameterPlasticityStressUpdate::consistentTangentOperator(stress_trial,
                                                                p_trial,
                                                                q_trial,
                                                                stress,
                                                                p,
                                                                q,
                                                                gaE,
                                                                smoothed_q,
                                                                Eijkl,
                                                                compute_full_tangent_operator,
                                                                cto);
}
