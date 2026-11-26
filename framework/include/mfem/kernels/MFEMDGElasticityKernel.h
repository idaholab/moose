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
 * - \langle \{ \sigma(\vec u) \cdot \hat n \}, [\vec v] \rangle
 * + \alpha \langle \{ \sigma(\vec v) \cdot \hat n \}, [\vec u] \rangle
 * + \kappa \langle h^{-1} \{ \lambda + 2 \mu \} [\vec u], [\vec v] \rangle
 * \f]
 */
class MFEMDGElasticityKernel : public MFEMKernel
{
public:
  static InputParameters validParams();

  MFEMDGElasticityKernel(const InputParameters & parameters);

  virtual mfem::BilinearFormIntegrator * createBFIntegrator() override;

  virtual bool isDGKernel() const override { return true; }

protected:
  int _fe_order;
  mfem::Coefficient & _lambda;
  mfem::Coefficient & _mu;
  mfem::real_t _alpha;
  mfem::real_t _kappa;
};

#endif
