//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CovarianceFunctionBase.h"

class LMC : public CovarianceFunctionBase
{
public:
  static InputParameters validParams();
  LMC(const InputParameters & parameters);

  //// Generates the Covariance Matrix given two points in the parameter space
  void computeCovarianceMatrix(RealEigenMatrix & K,
                               const RealEigenMatrix & x,
                               const RealEigenMatrix & xp,
                               const bool is_self_covariance) const override;

  /// Redirect dK/dhp for hyperparameter "hp"
  void computedKdhyper(RealEigenMatrix & dKdhp,
                       const RealEigenMatrix & x,
                       const std::string & hyper_param_name,
                       unsigned int ind) const override;

  void computeBMatrix(RealEigenMatrix & Bmat, const unsigned int exp_i) const;

  void computeAGradient(RealEigenMatrix & grad,
                        const unsigned int exp_i,
                        const unsigned int index) const;

  void computeLambdaGradient(RealEigenMatrix & grad,
                             const unsigned int exp_i,
                             const unsigned int index) const;

protected:
  const unsigned int _num_expansion_terms;
  const unsigned int _num_outputs;

  std::vector<const std::vector<Real> *> _a_coeffs;
  std::vector<const std::vector<Real> *> _lambdas;
};
