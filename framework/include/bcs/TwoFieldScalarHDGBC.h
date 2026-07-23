//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

class TwoFieldScalarHDGAssemblyHelper;

/**
 * Base boundary condition for two-field hybridized DG discretizations assembled with automatic
 * differentiation.
 */
class TwoFieldScalarHDGBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  TwoFieldScalarHDGBC(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeResidualAndJacobian() override;
  virtual void jacobianSetup() override;
  virtual const std::unordered_set<unsigned int> & getMatPropDependencies() const override;
  virtual bool getMaterialPropertyCalled() const override;

private:
  /// Computes the boundary residual data using the supplied assembly helper.
  virtual void compute(TwoFieldScalarHDGAssemblyHelper & helper) = 0;

  /// Returns the helper used for common two-field scalar HDG assembly.
  virtual TwoFieldScalarHDGAssemblyHelper & hdgHelper() = 0;
  const TwoFieldScalarHDGAssemblyHelper & hdgHelper() const;

  virtual ADReal computeQpResidual() override { mooseError("this will never be called"); }

  /// Element for which the complete AD Jacobian was most recently assembled.
  const Elem * _cached_elem;

  /// Side for which the complete AD Jacobian was most recently assembled.
  unsigned int _cached_side;
};

inline const TwoFieldScalarHDGAssemblyHelper &
TwoFieldScalarHDGBC::hdgHelper() const
{
  return const_cast<TwoFieldScalarHDGBC *>(this)->hdgHelper();
}
