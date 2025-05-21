//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IPHDGDirichletBC.h"
#include "NavierStokesStressIPHDGAssemblyHelper.h"

/**
 * Weakly imposes Dirichlet boundary conditions for a hybridized discretization of a Navier-Stokes
 * equation stress term
 */
class NavierStokesStressIPHDGDirichletBC : public IPHDGDirichletBC
{
public:
  static InputParameters validParams();
  NavierStokesStressIPHDGDirichletBC(const InputParameters & parameters);

protected:
  virtual IPHDGAssemblyHelper & iphdgHelper() override { return *_iphdg_helper; }

  /// The assembly helper providing the required IP-HDG method implementations
  std::unique_ptr<NavierStokesStressIPHDGAssemblyHelper> _iphdg_helper;
};
