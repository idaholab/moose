//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhaseFieldFractureSpectralSplitBase.h"

InputParameters
PhaseFieldFractureSpectralSplitBase::validParams()
{
  InputParameters params = PhaseFieldFractureBase::validParams();
  return params;
}

PhaseFieldFractureSpectralSplitBase::PhaseFieldFractureSpectralSplitBase(
    const InputParameters & parameters)
  : PhaseFieldFractureBase(parameters)
{
}

void
PhaseFieldFractureSpectralSplitBase::spectralSplit(const RankTwoTensor & r2t,
                                                   RankTwoTensor & r2t_pos,
                                                   RankFourTensor & P_pos) const
{
  RankTwoTensor eigvecs;
  std::vector<Real> eigvals(LIBMESH_DIM);
  P_pos = r2t.positiveProjectionEigenDecomposition(eigvals, eigvecs);

  RankTwoTensor eigvals_pos;
  eigvals_pos.fillFromInputVector(Macaulay(eigvals));
  r2t_pos = eigvecs * eigvals_pos * eigvecs.transpose();
}
