//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KernelBase.h"
#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"

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
class HybridizedKernel : public KernelBase
{
public:
  static InputParameters validParams();

  HybridizedKernel(const InputParameters & parameters);

  virtual void computeResidual() override final;
  virtual void computeJacobian() override final;
  virtual void computeOffDiagJacobian(unsigned int) override final;
  virtual void computeOffDiagJacobianScalar(unsigned int) override final;

  virtual void computeResidualAndJacobian() override final;

  /**
   * Here we compute the updates to the primal variables (solution and gradient) now that we have
   * the update to our dual (Lagrange multiplier) variable
   */
  void computePostLinearSolve();

  virtual void initialSetup() override;

  static const std::string lm_increment_vector_name;

protected:
  /**
   * Perform finite element assembly on the volumetric quadrature points
   */
  virtual void onElement() = 0;

  /**
   * Perform finite element assembly on external boundaries
   */
  virtual void onBoundary() = 0;

  /**
   * Perform finite element assembly on internal sides
   */
  virtual void onInternalSide() = 0;

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

  /// The auxiliary system
  SystemBase & _aux_sys;

  /**
   * The (ghosted) increment of the Lagrange multiplier vector. This will be used post-linear solve
   * (pre linesearch) to update the primal solution which resides in the auxiliary system
   */
  const NumericVector<Number> * _lm_increment;

  /// The current boundary ID
  const BoundaryID & _current_bnd_id;

  /// The face quadrature rule
  const QBase * const & _qrule_face;

  /// The physical locations of the quadrature points on the face
  const MooseArray<Point> & _q_point_face;

  /// transformed Jacobian weights on the face
  const MooseArray<Real> & _JxW_face;

  /// coordinate transformation on the face
  const MooseArray<Real> & _coord_face;

  /// face normals
  const MooseArray<Point> & _normals;

  /// Lagrange multiplier matrix and RHS after eliminating vector and scalar dofs
  DenseMatrix<Number> _K_libmesh;
  DenseVector<Number> _F_libmesh;

  /// Matrix data structures for on-diagonal coupling
  EigenMatrix _MixedMat, _MixedMatInv, _LMMat;
  /// Vector data structures
  EigenVector _MixedVec, _LMVec;
  /// Matrix data structures for off-diagonal coupling
  EigenMatrix _MixedLM, _LMMixed;

  /// The current neighbor
  const Elem * _neigh;

private:
  /**
   * Local finite element assembly
   */
  void assemble();

#ifndef NDEBUG
  /// The current side index
  const unsigned int & _current_side;
#endif

  /// Whether we are assembling the Lagrange multiplier residual and Jacobian
  bool _computing_global_data;
};

inline void
HybridizedKernel::computeResidual()
{
  mooseError("Hybridized kernels only implement computeResidualAndJacobian");
}

inline void
HybridizedKernel::computeJacobian()
{
  mooseError("Hybridized kernels only implement computeResidualAndJacobian");
}

inline void
HybridizedKernel::computeOffDiagJacobian(unsigned int)
{
  mooseError("Hybridized kernels only implement computeResidualAndJacobian");
}

inline void
HybridizedKernel::computeOffDiagJacobianScalar(unsigned int)
{
  mooseError("Hybridized kernels only implement computeResidualAndJacobian");
}

inline void
HybridizedKernel::computeResidualAndJacobian()
{
  _computing_global_data = true;
  assemble();
}

inline void
HybridizedKernel::computePostLinearSolve()
{
  _computing_global_data = false;
  assemble();
}

inline void
HybridizedKernel::initialSetup()
{
  KernelBase::initialSetup();
  _lm_increment = &_sys.getVector(lm_increment_vector_name);
}

inline void
HybridizedKernel::createIdentityResidual(const QBase & quadrature,
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
HybridizedKernel::createIdentityJacobian(const QBase & quadrature,
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
