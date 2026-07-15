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

class HybridizedDGAssemblyHelper;

/**
 * Base kernel for two-field hybridized DG discretizations assembled with automatic
 * differentiation.
 */
class HybridizedDGKernel : public HDGKernel
{
public:
  static InputParameters validParams();

  HybridizedDGKernel(const InputParameters & params);
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

protected:
  virtual HybridizedDGAssemblyHelper & hybridizedDGHelper() = 0;
  const HybridizedDGAssemblyHelper & hybridizedDGHelper() const;

  void compute();
  void computeOnSide();

  /// Used to assemble the complete AD Jacobian only once per element.
  const Elem * _cached_elem;
};

inline const HybridizedDGAssemblyHelper &
HybridizedDGKernel::hybridizedDGHelper() const
{
  return const_cast<HybridizedDGKernel *>(this)->hybridizedDGHelper();
}
