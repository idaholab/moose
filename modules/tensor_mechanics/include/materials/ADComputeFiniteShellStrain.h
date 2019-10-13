//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADComputeIncrementalShellStrain.h"
#include "libmesh/dense_matrix.h"

#define usingComputeFiniteShellStrainMembers usingComputeIncrementalShellStrainMembers;

// Forward Declarations
template <ComputeStage>
class ADComputeFiniteShellStrain;

declareADValidParams(ADComputeFiniteShellStrain);

template <ComputeStage compute_stage>
class ADComputeFiniteShellStrain : public ADComputeIncrementalShellStrain<compute_stage>
{
public:
  ADComputeFiniteShellStrain(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeProperties() override;
  virtual void computeBNLMatrix();
  virtual void computeNodeNormal() override;
  virtual void updatedxyz() override;
  virtual void updateGVectors() override;

  std::vector<ADMaterialProperty(DenseMatrix<Real>) *> _BNL;

  usingComputeIncrementalShellStrainMembers;
};
