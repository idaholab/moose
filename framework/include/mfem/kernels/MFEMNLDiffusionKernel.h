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

/**
 * \f[
 * (u \vec \nabla u, \vec \nabla v)
 * \f]
 */
class MFEMNLDiffusionIntegrator : public mfem::NonlinearFormIntegrator
{
public:
  MFEMNLDiffusionIntegrator(mfem::Coefficient & q,
                            const mfem::GridFunction * gf,
                            const mfem::IntegrationRule * ir = nullptr);

  virtual void AssembleElementVector(const mfem::FiniteElement & el,
                                     mfem::ElementTransformation & Tr,
                                     const mfem::Vector & elfun,
                                     mfem::Vector & elvect) override;
  virtual void AssembleElementGrad(const mfem::FiniteElement & el,
                                   mfem::ElementTransformation & Tr,
                                   const mfem::Vector & elfun,
                                   mfem::DenseMatrix & elmat) override;

protected:
  mfem::GradientGridFunctionCoefficient _grad_trial;
  mfem::DiffusionIntegrator _diffusion_integ;
  mfem::ConstantCoefficient _one{1.0};
  mfem::ScalarVectorProductCoefficient _product_coef_jac;
  mfem::MixedScalarWeakDivergenceIntegrator _weak_div_integ;
  mfem::SumIntegrator _sum{0};
};

/**
 * \f[
 * (u \vec \nabla u, \vec \nabla v)
 * \f]
 */
class MFEMNLDiffusionKernel : public MFEMKernel
{
public:
  static InputParameters validParams();

  MFEMNLDiffusionKernel(const InputParameters & parameters);

  virtual mfem::NonlinearFormIntegrator * createNLIntegrator() override;

protected:
  mfem::Coefficient & _coef;
};

#endif
