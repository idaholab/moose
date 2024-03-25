//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "StochasticToolsApp.h"
#include "MooseObject.h"

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

class OutputCovarianceBase : public MooseObject
{
public:
  static InputParameters validParams();
  OutputCovarianceBase(const InputParameters & parameters);

  /// Generates the B Covariance Matrix for capturing output covariances
  virtual void computeBCovarianceMatrix(RealEigenMatrix & B,
                                        const std::vector<Real> & latent) const = 0;

  /// Generates the full Covariance Matrix given two points in the parameter space
  virtual void computeFullCovarianceMatrix(RealEigenMatrix & kappa,
                                           const RealEigenMatrix & B,
                                           const RealEigenMatrix & K) const;

  /// Compute the gradient of the B matrix
  virtual void computeBGrad(RealEigenMatrix & BGrad,
                            const std::vector<Real> & latent,
                            const unsigned int & index) const = 0;

  /// Setup the number of latent params
  virtual unsigned int setupNumLatent(const unsigned int & num_outputs) const;

protected:
  void kron(const RealEigenMatrix & mat_A,
            const RealEigenMatrix & mat_B,
            RealEigenMatrix & mat_req) const;
};
