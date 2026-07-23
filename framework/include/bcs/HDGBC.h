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

class HDGAssemblyHelper;

/**
 * Base boundary condition for two-field hybridized DG discretizations assembled with automatic
 * differentiation.
 */
class HDGBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  HDGBC(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeResidualAndJacobian() override;
  virtual void jacobianSetup() override;
  virtual const std::unordered_set<unsigned int> & getMatPropDependencies() const override;
  virtual bool getMaterialPropertyCalled() const override;

protected:
  /// Computes the boundary residual data in the assembly helper.
  virtual void compute() = 0;

  /**
   * Returns the helper used for assembly. HDG boundary conditions are always helper-backed, so a
   * reference is used instead of the nullable kernel-helper pointer.
   */
  virtual HDGAssemblyHelper & hdgHelper() = 0;
  const HDGAssemblyHelper & hdgHelper() const;

  virtual ADReal computeQpResidual() override { mooseError("this will never be called"); }

  /// Element for which the complete AD Jacobian was most recently assembled.
  const Elem * _cached_elem;

  /// Side for which the complete AD Jacobian was most recently assembled.
  unsigned int _cached_side;
};

inline const HDGAssemblyHelper &
HDGBC::hdgHelper() const
{
  return const_cast<HDGBC *>(this)->hdgHelper();
}
