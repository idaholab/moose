/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef ASMBLOCK_H
#define ASMBLOCK_H

#include <vector>
#include "Moose.h"
#include "ParallelUniqueId.h"
#include "MooseVariable.h"
// libMesh
#include "dof_map.h"
#include "dense_matrix.h"
#include "dense_vector.h"
#include "coupling_matrix.h"

class SystemBase;

class AsmBlock
{
public:
  AsmBlock(SystemBase & sys, CouplingMatrix * & cm, THREAD_ID tid);
  virtual ~AsmBlock();

  void init();

  void prepare();
  void prepareNeighbor();
  void prepareBlock(unsigned int ivar, unsigned jvar, const std::vector<unsigned int> & dof_indices);

  void copyShapes(unsigned int var);
  void copyFaceShapes(unsigned int var);
  void copyNeighborShapes(unsigned int var);

  void addResidual(NumericVector<Number> & residual);
  void addResidualNeighbor(NumericVector<Number> & residual);

  /**
   * Takes the values that are currently in _sub_Re and appends them to the cached values.
   */
  void cacheResidual();

  /**
   * Takes the values that are currently in _sub_Ke and appends them to the cached values.
   */
  void cacheResidualNeighbor();

  /**
   * Adds the values that have been cached by calling cacheResidual() and or cacheResidualNeighbor() to the residual.
   *
   * Note that this will also clear the cache.
   */
  void addCachedResidual(NumericVector<Number> & residual);

  void setResidual(NumericVector<Number> & residual);
  void setResidualNeighbor(NumericVector<Number> & residual);

  void addJacobian(SparseMatrix<Number> & jacobian);
  void addJacobianBlock(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices);
  void addJacobianNeighbor(SparseMatrix<Number> & jacobian);
  void addJacobianNeighbor(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices, std::vector<unsigned int> & neighbor_dof_indices);

  /**
   * Takes the values that are currently in _sub_Kee and appends them to the cached values.
   */
  void cacheJacobian();

  /**
   * Takes the values that are currently in the neighbor Dense Matrices and appends them to the cached values.
   */
  void cacheJacobianNeighbor();

  /**
   * Adds the values that have been cached by calling cacheJacobian() and or cacheJacobianNeighbor() to the jacobian matrix.
   *
   * Note that this will also clear the cache.
   */
  void addCachedJacobian(SparseMatrix<Number> & jacobian);

  DenseVector<Number> & residualBlock(unsigned int var_num) { return _sub_Re[var_num]; }
  DenseVector<Number> & residualBlockNeighbor(unsigned int var_num) { return _sub_Rn[var_num]; }

  DenseMatrix<Number> & jacobianBlock(unsigned int ivar, unsigned int jvar) { return _sub_Kee[ivar][jvar]; }
  DenseMatrix<Number> & jacobianBlockNeighbor(Moose::DGJacobianType type, unsigned int ivar, unsigned int jvar);
  void cacheJacobianBlock(DenseMatrix<Number> & jac_block, std::vector<unsigned int> & idof_indices, std::vector<unsigned int> & jdof_indices, Real scaling_factor);

  std::vector<std::pair<unsigned int, unsigned int> > & couplingEntries() { return _cm_entry; }

  const std::vector<std::vector<Real> > & phi() { return _phi; }
  const std::vector<std::vector<RealGradient> > & gradPhi() { return _grad_phi; }
  const std::vector<std::vector<RealTensor> > & secondPhi() { return _second_phi; }

  const std::vector<std::vector<Real> > & phiFace() { return _phi_face; }
  const std::vector<std::vector<RealGradient> > & gradPhiFace() { return _grad_phi_face; }
  const std::vector<std::vector<RealTensor> > & secondPhiFace() { return _second_phi_face; }

  const std::vector<std::vector<Real> > & phiFaceNeighbor() { return _phi_face_neighbor; }
  const std::vector<std::vector<RealGradient> > & gradPhiFaceNeighbor() { return _grad_phi_face_neighbor; }
  const std::vector<std::vector<RealTensor> > & secondPhiFaceNeighbor() { return _second_phi_face_neighbor; }

protected:
  void addResidualBlock(NumericVector<Number> & residual, DenseVector<Number> & res_block, std::vector<unsigned int> & dof_indices, Real scaling_factor);
  void cacheResidualBlock(DenseVector<Number> & res_block, std::vector<unsigned int> & dof_indices, Real scaling_factor);

  void setResidualBlock(NumericVector<Number> & residual, DenseVector<Number> & res_block, std::vector<unsigned int> & dof_indices, Real scaling_factor);

  void addJacobianBlock(SparseMatrix<Number> & jacobian, DenseMatrix<Number> & jac_block, std::vector<unsigned int> & idof_indices, std::vector<unsigned int> & jdof_indices, Real scaling_factor);

  SystemBase & _sys;
  CouplingMatrix * & _cm;                                        ///< Reference to coupling matrix
  std::vector<std::pair<unsigned int, unsigned int> > _cm_entry; ///< Entries in the coupling matrix
  const DofMap & _dof_map;                                       ///< DOF map
  THREAD_ID _tid;                                                ///< Thread number (id)

  std::vector<DenseVector<Number> > _sub_Re;                     ///< residual contributions for each variable from the element
  std::vector<DenseVector<Number> > _sub_Rn;                     ///< residual contributions for each variable from the neighbor

  std::vector<std::vector<DenseMatrix<Number> > > _sub_Kee;      ///< jacobian contributions

  std::vector<std::vector<DenseMatrix<Number> > > _sub_Ken;      ///< jacobian contributions from the element and neighbor
  std::vector<std::vector<DenseMatrix<Number> > > _sub_Kne;      ///< jacobian contributions from the neighbor and element
  std::vector<std::vector<DenseMatrix<Number> > > _sub_Knn;      ///< jacobian contributions from the neighbor

  // Shape function values, gradients. second derivatives
  std::vector<std::vector<Real> > _phi;
  std::vector<std::vector<RealGradient> > _grad_phi;
  std::vector<std::vector<RealTensor> > _second_phi;

  std::vector<std::vector<Real> > _phi_face;
  std::vector<std::vector<RealGradient> > _grad_phi_face;
  std::vector<std::vector<RealTensor> > _second_phi_face;

  std::vector<std::vector<Real> > _phi_face_neighbor;
  std::vector<std::vector<RealGradient> > _grad_phi_face_neighbor;
  std::vector<std::vector<RealTensor> > _second_phi_face_neighbor;

  std::vector<Real> _cached_residual_values;                     /// Values cached by calling cacheResidual()
  std::vector<unsigned int> _cached_residual_rows;               /// Where the cached values should go

  unsigned int _max_cached_residuals;

  std::vector<Real> _cached_jacobian_values;                     /// Values cached by calling cacheJacobian()
  std::vector<unsigned int> _cached_jacobian_rows;               /// Row where the corresponding cached value should go
  std::vector<unsigned int> _cached_jacobian_cols;               /// Column where the corresponding cached value should go

  unsigned int _max_cached_jacobians;
};

#endif /* ASMBLOCK_H */
