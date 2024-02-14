//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HybridizedIntegratedBC.h"
#include "NavierStokesHybridizedInterface.h"

#include <vector>

template <typename>
class MooseVariableFE;
class MooseVariableScalar;
template <typename>
class MooseArray;
class Function;

/**
 * Weakly imposes Dirichlet boundary conditions for the velocity for a hybridized discretization of
 * the Navier-Stokes equations
 */
class NavierStokesHybridizedVelocityDirichletBC : public HybridizedIntegratedBC,
                                                  public NavierStokesHybridizedInterface
{
public:
  static InputParameters validParams();

  NavierStokesHybridizedVelocityDirichletBC(const InputParameters & parameters);

  virtual const MooseVariableBase & variable() const override { return _u_face_var; }

protected:
  virtual void onBoundary() override;

private:
  /// Dirichlet velocity
  std::array<const Function *, 3> _dirichlet_vel;

  friend class NavierStokesHybridizedInterface;
  friend class DiffusionHybridizedInterface;
};
