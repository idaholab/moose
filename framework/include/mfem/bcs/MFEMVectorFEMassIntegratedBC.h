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

#include "MFEMIntegratedBC.h"

namespace Moose::MFEM
{
class VectorFEMassIntegratedBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  VectorFEMassIntegratedBC(const InputParameters & parameters);

  /// Create MFEM integrator to apply to the LHS of the weak form. Ownership managed by the caller.
  virtual mfem::BilinearFormIntegrator * createBFIntegrator();

protected:
  mfem::Coefficient & _coef;
};

} // namespace Moose::MFEM
#endif
