//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"
#include "DiffusionLHDGAssemblyHelper.h"

/**
 * Weakly imposes Dirichlet boundary conditions for a hybridized discretization of diffusion
 */
class DiffusionLHDGDirichletBC : public IntegratedBC, public DiffusionLHDGAssemblyHelper
{
public:
  static InputParameters validParams();

  DiffusionLHDGDirichletBC(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void jacobianSetup() override;
  virtual void initialSetup() override;

protected:
  virtual Real computeQpResidual() override { mooseError("this will never be called"); }

private:
  /// Functor computing the Dirichlet boundary value
  const Moose::Functor<Real> & _dirichlet_val;

  /// A cache variable to prevent multiple computations of Jacobians
  unsigned int _cached_side;
};
