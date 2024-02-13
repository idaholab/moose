//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBCBase.h"
#include "HybridizedInterface.h"

/**
 * An integrated boundary condition for hybridized finite element formulations
 */
class HybridizedIntegratedBC : public IntegratedBCBase, public HybridizedInterface
{
public:
  static InputParameters validParams();

  HybridizedIntegratedBC(const InputParameters & parameters);

  virtual void computeResidual() override final;
  virtual void computeJacobian() override final;
  virtual void computeOffDiagJacobian(unsigned int) override final;
  virtual void computeOffDiagJacobianScalar(unsigned int) override final;
  virtual void computeResidualAndJacobian() override final;

protected:
  void createIdentityResidual(const MooseArray<std::vector<Real>> & phi,
                              const MooseArray<Number> & sol,
                              const std::size_t n_dofs,
                              const unsigned int i_offset);

  void createIdentityJacobian(const MooseArray<std::vector<Real>> & phi,
                              const std::size_t n_dofs,
                              const unsigned int ij_offset);

  /// normals at quadrature points
  const MooseArray<Point> & _normals;

  /// The auxiliary system
  SystemBase & _aux_sys;

  /// The current neighbor
  const Elem * _neigh = nullptr;

private:
  friend class HybridizedKernel;

  /**
   * Perform finite element assembly for this boundary condition
   */
  virtual void onBoundary() = 0;
};

inline void
HybridizedIntegratedBC::computeResidual()
{
  mooseError("Hybridized integrated bcs do not implement global data assembly methods");
}

inline void
HybridizedIntegratedBC::computeJacobian()
{
  mooseError("Hybridized integrated bcs do not implement global data assembly methods");
}

inline void
HybridizedIntegratedBC::computeOffDiagJacobian(unsigned int)
{
  mooseError("Hybridized integrated bcs do not implement global data assembly methods");
}

inline void
HybridizedIntegratedBC::computeOffDiagJacobianScalar(unsigned int)
{
  mooseError("Hybridized integrated bcs do not implement global data assembly methods");
}

inline void
HybridizedIntegratedBC::computeResidualAndJacobian()
{
  mooseError("Hybridized kernels only implement computeResidualAndJacobian");
}

inline void
HybridizedIntegratedBC::createIdentityResidual(const MooseArray<std::vector<Real>> & phi,
                                               const MooseArray<Number> & sol,
                                               const std::size_t n_dofs,
                                               const unsigned int i_offset)
{
  for (const auto qp : make_range(_qrule->n_points()))
    for (const auto i : make_range(n_dofs))
      _LMVec(i_offset + i) -= _JxW[qp] * phi[i][qp] * sol[qp];
}

inline void
HybridizedIntegratedBC::createIdentityJacobian(const MooseArray<std::vector<Real>> & phi,
                                               const std::size_t n_dofs,
                                               const unsigned int ij_offset)
{
  for (const auto qp : make_range(_qrule->n_points()))
    for (const auto i : make_range(n_dofs))
      for (const auto j : make_range(n_dofs))
        _LMMat(ij_offset + i, ij_offset + j) -= _JxW[qp] * phi[i][qp] * phi[j][qp];
}
