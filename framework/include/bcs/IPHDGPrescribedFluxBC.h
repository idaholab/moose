//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"
#include "ADFunctorInterface.h"

class IPHDGAssemblyHelper;

/**
 * Implements a prescribed flux for an IP-HDG discretization
 */
class IPHDGPrescribedFluxBC : public IntegratedBC, public ADFunctorInterface
{
public:
  static InputParameters validParams();

  IPHDGPrescribedFluxBC(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeResidualAndJacobian() override;
  virtual void jacobianSetup() override;

protected:
  virtual IPHDGAssemblyHelper & iphdgHelper() = 0;

  /**
   * compute the AD residuals
   */
  void compute();

  virtual Real computeQpResidual() override { mooseError("this will never be called"); }

  /// Prescribed normal flux along the boundary. The default is 0 for a natural boundary
  /// condition
  const Moose::Functor<Real> & _normal_flux;

  /// A data member used for determining when to compute the Jacobian
  const Elem * _my_elem;

  /// A cache variable to prevent multiple computations of Jacobians
  unsigned int _my_side;
};
