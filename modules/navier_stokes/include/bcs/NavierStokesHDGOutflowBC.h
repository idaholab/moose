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
class NavierStokesHDGOutflowBC : public IntegratedBC, public NavierStokesHDGAssemblyHelper
{
public:
  static InputParameters validParams();

  NavierStokesHDGOutflowBC(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void jacobianSetup() override;
  virtual void initialSetup() override;

protected:
  virtual Real computeQpResidual() override { mooseError("this will never be called"); }

  /// A cache variable to prevent multiple computations of Jacobians
  unsigned int _my_side;
};
