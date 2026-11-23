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
 * (F, \vec \nabla \vec v)
 * \f]
 * where each component of \f$ \vec v \f$ lies in the same scalar FE space, as for an H1 vector
 * variable, and \f$ F \f$ is either a matrix coefficient or the isotropic tensor \f$ f I \f$ for
 * a scalar coefficient \f$ f \f$, giving \f$ (f, \vec \nabla \cdot \vec v) \f$.
 */
class MFEMVectorDomainLFGradKernel : public MFEMKernel
{
public:
  static InputParameters validParams();

  MFEMVectorDomainLFGradKernel(const InputParameters & parameters);

  virtual mfem::LinearFormIntegrator * createLFIntegrator() override;

protected:
  /// The tensor coefficient F, flattened into the layout mfem::VectorDomainLFGradIntegrator expects
  mfem::VectorCoefficient & _flattened_tensor_coef;

private:
  /// Return the matrix coefficient F, building f I from the scalar coefficient f if no matrix
  /// coefficient is provided
  mfem::MatrixCoefficient & getTensorCoefficient();
};

#endif
