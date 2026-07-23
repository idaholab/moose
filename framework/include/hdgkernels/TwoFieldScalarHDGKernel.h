//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HDGKernel.h"

class TwoFieldScalarHDGAssemblyHelper;

/**
 * Base kernel for helper-backed HDG discretizations with an element-interior scalar and scalar
 * facet trace.
 */
class TwoFieldScalarHDGKernel : public HDGKernel
{
public:
  static InputParameters validParams();

  TwoFieldScalarHDGKernel(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeResidualAndJacobian() override;
  virtual void jacobianSetup() override;
  virtual void computeResidualOnSide() override;
  virtual void computeJacobianOnSide() override;
  virtual void computeResidualAndJacobianOnSide() override;

  virtual std::set<std::string> additionalROVariables() override;
  virtual const std::unordered_set<unsigned int> & getMatPropDependencies() const override;
  virtual bool getMaterialPropertyCalled() const override;

private:
  /// Returns the helper used for common two-field scalar HDG assembly.
  virtual TwoFieldScalarHDGAssemblyHelper & hdgHelper() = 0;
  const TwoFieldScalarHDGAssemblyHelper & hdgHelper() const;

  /// Assembles the element-interior residual data.
  void compute(TwoFieldScalarHDGAssemblyHelper & helper);

  /// Assembles the face residual data.
  void computeOnSide(TwoFieldScalarHDGAssemblyHelper & helper);

  /// Used to assemble the complete AD Jacobian only once per element.
  const Elem * _cached_elem;
};

inline const TwoFieldScalarHDGAssemblyHelper &
TwoFieldScalarHDGKernel::hdgHelper() const
{
  return const_cast<TwoFieldScalarHDGKernel *>(this)->hdgHelper();
}
