//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IPHDGBC.h"

class IPHDGAssemblyHelper;

/**
 * Base class for imposing Dirichlet boundary conditions for interior penalty hybridizable
 * discontinuous Galerkin methods
 */
class IPHDGDirichletBC : public IPHDGBC
{
public:
  static InputParameters validParams();
  IPHDGDirichletBC(const InputParameters & parameters);

protected:
  /**
   * compute the AD residuals
   */
  virtual void compute() override;

private:
  /// Functor computing the Dirichlet boundary value
  const Moose::Functor<Real> & _dirichlet_val;
};
