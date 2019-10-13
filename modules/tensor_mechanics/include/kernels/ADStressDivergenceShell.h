//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "libmesh/quadrature_gauss.h"

#define usingADStressDivergenceShellMembers usingKernelMembers;

// Forward Declarations
template <ComputeStage>
class ADStressDivergenceShell;

namespace libMesh
{
class QGauss;
}

declareADValidParams(ADStressDivergenceShell);

/**
 * ADStressDivergenceShell computes the stress divergence term for shell elements.
 **/

template <ComputeStage compute_stage>
class ADStressDivergenceShell : public ADKernel<compute_stage>
{
public:
  ADStressDivergenceShell(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  const unsigned int _component;
  const bool _large_strain;

  std::vector<const ADMaterialProperty(RankTwoTensor) *> _stress;
  std::vector<const MaterialProperty<RankTwoTensor> *> _stress_old;
  std::vector<const ADMaterialProperty(DenseMatrix<Real>) *> _B_mat;
  std::vector<const ADMaterialProperty(DenseMatrix<Real>) *> _B_NL;
  std::vector<const ADMaterialProperty(Real) *> _Jmap;

  /// Quadrature rule in the out of plane direction
  std::unique_ptr<QGauss> _t_qrule;

  /// Quadrature points in the out of plane direction in isoparametric coordinate system
  std::vector<Real> _t_weights;

  /// Qrule weights in isoparametric coordinate system
  std::vector<Real> _q_weights;

  /// qp index in out of plane direction
  unsigned int _qp_z;

  usingKernelMembers;
};
