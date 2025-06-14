//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#pragma once
#include "MFEMKernel.h"

/*
 * \f[
 * (\sigma \nabla q, \nabla q')
 * \f]
 */
class MFEMDiffusionKernel : public MFEMKernel
{
public:
  static InputParameters validParams();

  MFEMDiffusionKernel(const InputParameters & parameters);

  virtual mfem::BilinearFormIntegrator * createBFIntegrator() override;

protected:
  const MFEMScalarCoefficientName & _coef_name;
  mfem::Coefficient & _coef;
};

#endif
