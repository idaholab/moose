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
  RealVectorValue getDirichletVelocity(const unsigned int qp) const;

  void pressureDirichletResidual(const unsigned int i_offset);

  void vectorDirichletResidual(const unsigned int i_offset, const unsigned int vel_component);

  void scalarDirichletResidual(const unsigned int i_offset,
                               const MooseArray<Gradient> & vector_sol,
                               const MooseArray<Number> & scalar_sol,
                               const unsigned int vel_component);

  void scalarDirichletJacobian(const unsigned int i_offset,
                               const unsigned int vector_j_offset,
                               const unsigned int scalar_j_offset,
                               const unsigned int p_j_offset,
                               const unsigned int vel_component);
  /// Dirichlet velocity
  std::vector<const Function *> _dirichlet_vel;

  friend class NavierStokesHybridizedInterface;
};
