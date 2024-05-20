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
#include "HDGData.h"
#include "ADFunctorInterface.h"
#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"

template <typename>
class MooseObjectWarehouse;
class HDGIntegratedBC;

/**
 * A kernel for hybridized finite element formulations
 */
class HDGKernel : public KernelBase, virtual public HDGData, public ADFunctorInterface
{
public:
  static InputParameters validParams();

  HDGKernel(const InputParameters & parameters);

  virtual void computeResidual() override final;
  virtual void computeJacobian() override final;
  virtual void computeOffDiagJacobian(unsigned int) override final;
  virtual void computeOffDiagJacobianScalar(unsigned int) override final;

  virtual void computeResidualAndJacobian() override final;

  virtual void initialSetup() override;

  /**
   * Here we compute the updates to the primal variables (solution and gradient) now that we have
   * the update to our dual (Lagrange multiplier) variable
   */
  void computePostLinearSolve();

  /// The name of the \p NumericVector that holds the changes in all the global (dual) degrees of
  /// freedom from one Newton iteration to the next
  static const std::string lm_increment_vector_name;

protected:
  /**
   * Perform finite element assembly on the volumetric quadrature points
   */
  virtual void onElement() = 0;

  /**
   * Perform finite element assembly on internal sides
   */
  virtual void onInternalSide() = 0;

  /**
   * Whether we are currently assembling to prepare for the global trace degree of freedom solve
   */
  bool preparingForSolve() const { return _preparing_for_solve; }

  /// The auxiliary system which holds the primal variables. We will update the primal variable
  /// degrees of freedom based off the Lagrange multiplier increment computed by the linear solve
  /// This update is performed on every nonlinear (for example Newton) iteration.
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

  /// transformed Jacobian weights on the current element face
  const MooseArray<Real> & _JxW_face;

  /// coordinate transformation on the face
  const MooseArray<Real> & _coord_face;

  /// face normals
  const MooseArray<Point> & _normals;

  /// Containers for the global degree of freedom numbers for primal and LM variables
  /// respectively. These data members should be set in the derived class
  std::vector<dof_id_type> _primal_dof_indices;
  std::vector<dof_id_type> _lm_dof_indices;

  /// The current neighbor
  const Elem * _neigh;

private:
  /**
   * Local assembly for an element and its sides
   */
  void assemble();

  /*
   * Add-in HDGData for element sides with boundary conditions
   */
  void addBCData(const HDGIntegratedBC & hibc);

  /// The current side index
  const unsigned int & _current_side;

  /// Whether we are assembling the Lagrange multiplier residual and Jacobian
  bool _preparing_for_solve;

  /// The warehouse holding the hybridized integrated boundary conditions
  MooseObjectWarehouse<HDGIntegratedBC> & _hibc_warehouse;

  /// Lagrange multiplier matrix and RHS after eliminating vector and scalar dofs
  DenseMatrix<Number> _K_libmesh;
  DenseVector<Number> _F_libmesh;

  /// Primal matrix inverse
  EigenMatrix _PrimalMatInv;

  // local degree of freedom increment values
  std::vector<Number> _lm_increment_dof_values;
  std::vector<Number> _primal_increment_dof_values;
  EigenVector _LMIncrement, _PrimalIncrement;
};

inline void
HDGKernel::computeResidual()
{
  mooseError("HDGKernels should not need individual residual evaluations");
}

inline void
HDGKernel::computeJacobian()
{
  mooseError("HDG kernels only implement computeResidualAndJacobian");
}

inline void
HDGKernel::computeOffDiagJacobian(unsigned int)
{
  mooseError("HDG kernels only implement computeResidualAndJacobian");
}

inline void
HDGKernel::computeOffDiagJacobianScalar(unsigned int)
{
  mooseError("HDG kernels only implement computeResidualAndJacobian");
}

inline void
HDGKernel::computeResidualAndJacobian()
{
  _preparing_for_solve = true;
  assemble();
}

inline void
HDGKernel::computePostLinearSolve()
{
  _preparing_for_solve = false;
  assemble();
}
