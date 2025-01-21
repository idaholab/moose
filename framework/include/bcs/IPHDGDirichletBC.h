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
 * Weakly imposes Dirichlet boundary conditions for a hybridized discretization of diffusion
 */
class IPHDGDirichletBC : public IntegratedBC, public ADFunctorInterface
{
public:
  static InputParameters validParams();
  IPHDGDirichletBC(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeResidualAndJacobian() override;

  virtual void jacobianSetup() override;

  virtual IPHDGAssemblyHelper & iphdgHelper() = 0;

protected:
  /**
   * compute the AD residuals
   */
  void compute();

  virtual Real computeQpResidual() override { mooseError("this will never be called"); }

private:
  /// Functor computing the Dirichlet boundary value
  const Moose::Functor<Real> & _dirichlet_val;

  /// A data member used for determining when to compute the Jacobian
  const Elem * _my_elem;

  /// A cache variable to prevent multiple computations of Jacobians
  unsigned int _my_side;
};
