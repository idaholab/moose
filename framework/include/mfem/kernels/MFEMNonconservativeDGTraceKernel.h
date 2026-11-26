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

#include "MFEMDGTraceKernel.h"

/**
 * \f[
 * -\alpha \langle \rho_u (\vec Q \cdot \hat n) \{v\}, [u] \rangle
 * + \beta \langle \rho_u |\vec Q \cdot \hat n| [v], [u] \rangle
 * \f]
 */
class MFEMNonconservativeDGTraceKernel : public MFEMDGTraceKernel
{
public:
  static InputParameters validParams();

  MFEMNonconservativeDGTraceKernel(const InputParameters & parameters);

  virtual mfem::BilinearFormIntegrator * createBFIntegrator() override;
};

#endif
