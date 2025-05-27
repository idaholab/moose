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

class IPHDGAssemblyHelper;

/**
 * Implements a prescribed flux for an IP-HDG discretization
 */
class IPHDGBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  IPHDGBC(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeResidualAndJacobian() override;
  virtual void jacobianSetup() override;
  virtual const std::unordered_set<unsigned int> & getMatPropDependencies() const override;

protected:
  /**
   * compute the AD residuals
   */
  virtual void compute() = 0;

  virtual IPHDGAssemblyHelper & iphdgHelper() = 0;
  const IPHDGAssemblyHelper & iphdgHelper() const;

  virtual ADReal computeQpResidual() override { mooseError("this will never be called"); }

  /// A data member used for determining when to compute the Jacobian
  const Elem * _my_elem;

  /// A cache variable to prevent multiple computations of Jacobians
  unsigned int _my_side;
};

inline const IPHDGAssemblyHelper &
IPHDGBC::iphdgHelper() const
{
  return const_cast<IPHDGBC *>(this)->iphdgHelper();
}
