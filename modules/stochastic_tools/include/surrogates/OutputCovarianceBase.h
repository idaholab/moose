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
                                           const RealEigenMatrix & K) const = 0;

  // /// Used for outputting Hyper-parameter settings
  // virtual void
  // buildHyperParamMap(std::unordered_map<std::string, Real> & map,
  //                    std::unordered_map<std::string, std::vector<Real>> & vec_map) const;

  // /// Used for outputting additional Hyper-parameter settings in derived
  // virtual void buildAdditionalHyperParamMap(
  //     std::unordered_map<std::string, Real> & /*map*/,
  //     std::unordered_map<std::string, std::vector<Real>> & /*vec_map*/) const {};

  // /// Used for outputting Hyper-parameter settings for use in surrogate
  // virtual void loadHyperParamMap(std::unordered_map<std::string, Real> & map,
  //                                std::unordered_map<std::string, std::vector<Real>> & vec_map);

  // /// Used for outputting Hyper-parameter settings for use in surrogate for derived
  // virtual void
  // loadAdditionalHyperParamMap(std::unordered_map<std::string, Real> & /*map*/,
  //                             std::unordered_map<std::string, std::vector<Real>> & /*vec_map*/){};

  /// Compute the gradient of the B matrix
  virtual void computeBGrad(RealEigenMatrix & BGrad,
                            const std::vector<Real> & latent,
                            const unsigned int & index) const = 0;

  /// Setup the number of latent params
  virtual unsigned int setupNumLatent(const unsigned int & num_outputs) const;

  // virtual bool isTunable(std::string name) const;

  // virtual void getTuningData(std::string name, unsigned int & size, Real & min, Real & max) const;

// protected:
  // /// lengh factor (\ell) for the kernel, in vector form for multiple parameters
  // std::vector<Real> _length_factor;

  // /// signal variance (\sigma_f^2)
  // Real _sigma_f_squared;

  // /// noise variance (\sigma_n^2)
  // Real _sigma_n_squared;

  // /// list of tunable hyper-parameters
  // std::unordered_set<std::string> _tunable_hp;
};
