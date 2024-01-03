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

#include <Eigen/Dense>

#ifdef LIBMESH_USE_COMPLEX_NUMBERS
typedef Eigen::MatrixXcd EigenMatrix;
typedef Eigen::VectorXcd EigenVector;
#else
typedef Eigen::MatrixXd EigenMatrix;
typedef Eigen::VectorXd EigenVector;
#endif

/**
 * A kernel for mixed dual finite element formulations
 */
class HybridizedIntegratedBC : public IntegratedBCBase
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
  void createIdentityResidual(const QBase & quadrature,
                              const std::vector<Real> & JxW_local,
                              const std::vector<std::vector<Real>> & phi,
                              const std::vector<Number> & sol,
                              const std::size_t n_dofs,
                              const unsigned int i_offset);

  void createIdentityJacobian(const QBase & quadrature,
                              const std::vector<Real> & JxW_local,
                              const std::vector<std::vector<Real>> & phi,
                              const std::size_t n_dofs,
                              const unsigned int ij_offset);

  /// Matrix data structures for on-diagonal coupling
  EigenMatrix _MixedMat, _MixedMatInv, _LMMat;
  /// Vector data structures
  EigenVector _MixedVec, _LMVec;
  /// Matrix data structures for off-diagonal coupling
  EigenMatrix _MixedLM, _LMMixed;

private:
  /**
   * Local finite element assembly
   */
  virtual void assemble() = 0;

  friend class HybridizedKernel;
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
HybridizedIntegratedBC::createIdentityResidual(const QBase & quadrature,
                                               const std::vector<Real> & JxW_local,
                                               const std::vector<std::vector<Real>> & phi,
                                               const std::vector<Number> & sol,
                                               const std::size_t n_dofs,
                                               const unsigned int i_offset)
{
  for (const auto qp : make_range(quadrature.n_points()))
    for (const auto i : make_range(n_dofs))
      _LMVec(i_offset + i) -= JxW_local[qp] * phi[i][qp] * sol[qp];
}

inline void
HybridizedIntegratedBC::createIdentityJacobian(const QBase & quadrature,
                                               const std::vector<Real> & JxW_local,
                                               const std::vector<std::vector<Real>> & phi,
                                               const std::size_t n_dofs,
                                               const unsigned int ij_offset)
{
  for (const auto qp : make_range(quadrature.n_points()))
    for (const auto i : make_range(n_dofs))
      for (const auto j : make_range(n_dofs))
        _LMMat(ij_offset + i, ij_offset + j) -= JxW_local[qp] * phi[i][qp] * phi[j][qp];
}
