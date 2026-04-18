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

namespace Moose::MFEM
{
/**
 * \f[
 * (k(u) \vec \nabla u, \vec \nabla v)
 * \f]
 */
class NLDiffusionKernel : public Kernel
{
public:
  static InputParameters validParams();

  NLDiffusionKernel(const InputParameters & parameters);

  virtual mfem::NonlinearFormIntegrator * createNLIntegrator() override;

protected:
  mfem::Coefficient & _k_coef;
  mfem::Coefficient & _dk_du_coef;
  mfem::ParGridFunction & _trial_var;
};

} // namespace Moose::MFEM
#endif
