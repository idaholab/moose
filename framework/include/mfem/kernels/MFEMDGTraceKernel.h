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
 * \alpha \langle \rho_u (\vec Q \cdot \hat n) \{u\}, [v] \rangle
 * + \beta \langle \rho_u |\vec Q \cdot \hat n| [u], [v] \rangle
 * \f]
 */
class MFEMDGTraceKernel : public MFEMKernel
{
public:
  static InputParameters validParams();

  MFEMDGTraceKernel(const InputParameters & parameters);

  virtual mfem::BilinearFormIntegrator * createBFIntegrator() override;

  virtual bool isDGKernel() const override { return true; }

protected:
  /// Scalar coefficient rho, evaluated on each face in the element the velocity points into
  mfem::Coefficient & _coef;
  /// Velocity coefficient Q
  mfem::VectorCoefficient & _vec_coef;
  mfem::real_t _alpha;
  mfem::real_t _beta;
};

#endif
