//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "ADFunctorInterface.h"

class IPHDGAssemblyHelper;

/**
 * Implements the diffusion equation for a interior penalty hybridized discretization
 */
class IPHDGKernel : public Kernel, public ADFunctorInterface
{
public:
  static InputParameters validParams();

  IPHDGKernel(const InputParameters & params);
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeResidualAndJacobian() override;
  virtual void jacobianSetup() override;

  virtual std::set<std::string> additionalVariablesCovered() override;

protected:
  virtual IPHDGAssemblyHelper & iphdgHelper() = 0;

  /**
   * compute the AD residuals
   */
  void compute();

  virtual Real computeQpResidual() override { mooseError("this will never be called"); }

  /// Neighbor element pointer
  const Elem * _neigh;

  /// The current side index
  const unsigned int & _current_side;

  /// A data member used for determining when to compute the Jacobian
  const Elem * _my_elem;

  /// The assembly helper providing the required IP-HDG method implementations
  std::unique_ptr<IPHDGAssemblyHelper> _iphdg_helper;
};
