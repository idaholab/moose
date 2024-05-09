//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HDGIntegratedBC.h"
#include "NavierStokesHDGAssemblyHelper.h"

#include <vector>

template <typename>
class MooseVariableFE;
class MooseVariableScalar;
template <typename>
class MooseArray;
class Function;

/**
 * Implements an outflow boundary condition for use with a hybridized discretization of the
 * incompressible Navier-Stokes equations
 */
class NavierStokesHDGOutflowBC : public HDGIntegratedBC, public NavierStokesHDGAssemblyHelper
{
public:
  static InputParameters validParams();

  NavierStokesHDGOutflowBC(const InputParameters & parameters);

  virtual const MooseVariableBase & variable() const override { return _u_face_var; }

protected:
  virtual void onBoundary() override;
};
