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

class IPHDGAssemblyHelper;

/**
 * Base kernel for implementing an interior penalty hybridized discretization
 */
class IPHDGKernel : public HDGKernel
{
public:
  static InputParameters validParams();

  IPHDGKernel(const InputParameters & params);
  virtual void computeResidual() override;
  /**
   * Compute this object's entire element interior Jacobian, both on- and off-diagonal
   */
  virtual void computeJacobian() override;
  /**
   * Forwards to \p computeJacobian() the first time this is called for a given element
   */
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeResidualAndJacobian() override;
  virtual void jacobianSetup() override;
  virtual void computeResidualOnSide() override;
  virtual void computeJacobianOnSide() override;
  virtual void computeResidualAndJacobianOnSide() override;

  virtual std::set<std::string> additionalVariablesCovered() override;
  virtual const std::unordered_set<unsigned int> & getMatPropDependencies() const override;

protected:
  virtual IPHDGAssemblyHelper & iphdgHelper() = 0;
  const IPHDGAssemblyHelper & iphdgHelper() const;

  /**
   * compute the AD residuals on the element interior
   */
  void compute();

  /**
   * compute the AD residuals on the element sides
   */
  void computeOnSide();

  /// A data member used for determining when to compute the Jacobian
  const Elem * _cached_elem;
};

inline const IPHDGAssemblyHelper &
IPHDGKernel::iphdgHelper() const
{
  return const_cast<IPHDGKernel *>(this)->iphdgHelper();
}
