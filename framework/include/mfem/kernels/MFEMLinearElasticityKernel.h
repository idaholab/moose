//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once
#include "MFEMKernel.h"

// clang-format off
/**
 * \f[
 * (c_{ikjl} \nabla u_j, \nabla v_i),
 * c_{ikjl} = \lambda \delta_{ik} \delta_{jl} + \mu (\delta_{ij} \delta_{kl} + \delta_{il} \delta_{jk}),
 * \lambda = (E\nu)/((1-2\nu)(1+\nu)),
 * \mu = E/(2(1+\nu)),
 * \f]
 * E is Young's modulus,
 * \f$ \nu \f$ is Poisson's ratio
 */
// clang-format on
class MFEMLinearElasticityKernel : public MFEMKernel
{
public:
  static InputParameters validParams();

  MFEMLinearElasticityKernel(const InputParameters & parameters);

  virtual std::pair<mfem::BilinearFormIntegrator *, mfem::BilinearFormIntegrator *> createBFIntegrator() override;

protected:
  mfem::Coefficient & _lambda;
  mfem::Coefficient & _mu;

  const MFEMScalarCoefficientName & _lambda_imag_name;
  const MFEMScalarCoefficientName & _mu_imag_name;
  mfem::Coefficient & _lambda_imag;
  mfem::Coefficient & _mu_imag;

};

#endif
